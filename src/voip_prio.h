#ifndef VOIP_PRIO_H
#define VOIP_PRIO_H

#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>

// =========================
// Configuration Macros
// =========================
#define VOIP_PRIO_DEFAULT_SIP_PORT 5060
#define VOIP_PRIO_DEFAULT_RTP_PORT_START 16384
#define VOIP_PRIO_DEFAULT_RTP_PORT_END   32767
#define VOIP_PRIO_MARK 0x1A2B3C4D  // Unique mark for VoIP packets
#define VOIP_PRIO_SYSFS_DIR "voip_prio"

// =========================
// VoIP Prioritizer Config
// =========================
struct voip_prio_config {
    u16 sip_port;           // SIP signaling port
    u16 rtp_port_start;     // RTP port range start
    u16 rtp_port_end;       // RTP port range end
    u32 priority_mark;      // skb->mark value for VoIP packets
    u32 congestion_thresh;  // Congestion threshold (packets in queue)
    spinlock_t lock;        // Protects config updates
};

// =========================
// Function Prototypes
// =========================
int voip_prio_nf_init(struct voip_prio_config *cfg);
void voip_prio_nf_exit(void);
int voip_prio_qdisc_init(void);
void voip_prio_qdisc_exit(void);
int voip_prio_sysfs_init(struct voip_prio_config *cfg);
void voip_prio_sysfs_exit(void);

#endif // VOIP_PRIO_H 