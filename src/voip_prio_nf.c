#include "voip_prio.h"
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/netdevice.h>
#include <linux/jiffies.h>

static struct nf_hook_ops voip_nfho;
static struct voip_prio_config *cfg_ptr;

// Token-bucket state
static unsigned long last_token_time = 0;
static u32 tokens = 0;
#define TOKEN_BUCKET_RATE 1000 // tokens per second
#define TOKEN_BUCKET_BURST 200 // max tokens

// Helper: Check if UDP port is in RTP range
static inline bool is_rtp_port(u16 port, struct voip_prio_config *cfg) {
    return port >= cfg->rtp_port_start && port <= cfg->rtp_port_end;
}

// Helper: Check if packet is SIP (UDP/TCP 5060)
static inline bool is_sip_port(u16 port, struct voip_prio_config *cfg) {
    return port == cfg->sip_port;
}

// Helper: Check if congestion is present on the outgoing device
static bool is_congested(const struct nf_hook_state *state, struct voip_prio_config *cfg) {
    struct net_device *dev = state->out;
    if (!dev)
        return false;
    // Use tx_queue_len as a proxy for congestion
    return (dev->qdisc && dev->qdisc->q.qlen >= cfg->congestion_thresh);
}

// Token-bucket logic: returns true if a token is available
static bool token_bucket_allow(void) {
    unsigned long now = jiffies;
    unsigned long elapsed = now - last_token_time;
    u32 add_tokens = (elapsed * TOKEN_BUCKET_RATE) / HZ;
    if (add_tokens > 0) {
        tokens = min(tokens + add_tokens, (u32)TOKEN_BUCKET_BURST);
        last_token_time = now;
    }
    if (tokens > 0) {
        tokens--;
        return true;
    }
    return false;
}

// Main packet inspection and marking function
static unsigned int voip_nf_hookfn(void *priv,
                                  struct sk_buff *skb,
                                  const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct udphdr *udph;
    struct tcphdr *tcph;
    u16 sport, dport;
    bool is_voip = false;

    if (!skb)
        return NF_ACCEPT;

    iph = ip_hdr(skb);
    if (!iph)
        return NF_ACCEPT;

    if (iph->protocol == IPPROTO_UDP) {
        udph = udp_hdr(skb);
        sport = ntohs(udph->source);
        dport = ntohs(udph->dest);
        if (is_rtp_port(sport, cfg_ptr) || is_rtp_port(dport, cfg_ptr) ||
            is_sip_port(sport, cfg_ptr) || is_sip_port(dport, cfg_ptr)) {
            is_voip = true;
        }
    } else if (iph->protocol == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        sport = ntohs(tcph->source);
        dport = ntohs(tcph->dest);
        if (is_sip_port(sport, cfg_ptr) || is_sip_port(dport, cfg_ptr)) {
            is_voip = true;
        }
    }

    if (is_voip) {
        // Only mark if congestion is detected and token-bucket allows
        if (is_congested(state, cfg_ptr) && token_bucket_allow()) {
            skb->mark = cfg_ptr->priority_mark;
        }
    }
    return NF_ACCEPT;
}

// Netfilter hook registration
int voip_prio_nf_init(struct voip_prio_config *cfg)
{
    cfg_ptr = cfg;
    voip_nfho.hook = voip_nf_hookfn;
    voip_nfho.pf = NFPROTO_IPV4;
    voip_nfho.hooknum = NF_INET_POST_ROUTING;
    voip_nfho.priority = NF_IP_PRI_FIRST;
    return nf_register_net_hook(&init_net, &voip_nfho);
}

void voip_prio_nf_exit(void)
{
    nf_unregister_net_hook(&init_net, &voip_nfho);
} 