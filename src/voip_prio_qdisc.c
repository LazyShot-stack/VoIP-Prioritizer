#include "voip_prio.h"
#include <linux/netdevice.h>
#include <net/pkt_sched.h>

// This is a stub for a custom qdisc. For a full implementation, see Linux qdisc docs.
// For now, we rely on skb->mark and recommend using 'tc' to map marks to prio qdisc.

int voip_prio_qdisc_init(void)
{
    // In a full implementation, register a custom qdisc here.
    printk(KERN_INFO "[voip_prio] (Stub) qdisc init: use tc to map skb->mark to prio qdisc\n");
    return 0;
}

void voip_prio_qdisc_exit(void)
{
    // Unregister custom qdisc if implemented
    printk(KERN_INFO "[voip_prio] (Stub) qdisc exit\n");
} 