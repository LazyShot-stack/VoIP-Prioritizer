#include "voip_prio.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("World's Best Developer");
MODULE_DESCRIPTION("Kernel-Level Traffic Prioritizer for VoIP");
MODULE_VERSION("1.0");

// Global configuration instance
static struct voip_prio_config voip_cfg = {
    .sip_port = VOIP_PRIO_DEFAULT_SIP_PORT,
    .rtp_port_start = VOIP_PRIO_DEFAULT_RTP_PORT_START,
    .rtp_port_end = VOIP_PRIO_DEFAULT_RTP_PORT_END,
    .priority_mark = VOIP_PRIO_MARK,
    .congestion_thresh = 100, // Default threshold
};

// =========================
// Module Initialization
// =========================
static int __init voip_prio_init(void)
{
    int ret;
    spin_lock_init(&voip_cfg.lock);
    printk(KERN_INFO "[voip_prio] Initializing VoIP Prioritizer Module\n");

    ret = voip_prio_nf_init(&voip_cfg);
    if (ret) {
        printk(KERN_ERR "[voip_prio] Netfilter hook init failed\n");
        return ret;
    }
    ret = voip_prio_qdisc_init();
    if (ret) {
        voip_prio_nf_exit();
        printk(KERN_ERR "[voip_prio] Qdisc init failed\n");
        return ret;
    }
    ret = voip_prio_sysfs_init(&voip_cfg);
    if (ret) {
        voip_prio_qdisc_exit();
        voip_prio_nf_exit();
        printk(KERN_ERR "[voip_prio] Sysfs init failed\n");
        return ret;
    }
    printk(KERN_INFO "[voip_prio] Module loaded successfully\n");
    return 0;
}

// =========================
// Module Cleanup
// =========================
static void __exit voip_prio_exit(void)
{
    voip_prio_sysfs_exit();
    voip_prio_qdisc_exit();
    voip_prio_nf_exit();
    printk(KERN_INFO "[voip_prio] Module unloaded\n");
}

module_init(voip_prio_init);
module_exit(voip_prio_exit); 