#include "voip_prio.h"
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/rcupdate.h>

static struct kobject *voip_kobj;
static struct voip_prio_config *cfg_ptr;

// Helper macro for attribute show/store
#define VOIP_ATTR_RW(_name) \
static ssize_t _name##_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
    ssize_t ret; \
    rcu_read_lock(); \
    ret = sprintf(buf, "%u\n", cfg_ptr->_name); \
    rcu_read_unlock(); \
    return ret; \
} \
static ssize_t _name##_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) { \
    unsigned long val; \
    if (kstrtoul(buf, 10, &val) == 0) { \
        spin_lock(&cfg_ptr->lock); \
        cfg_ptr->_name = (typeof(cfg_ptr->_name))val; \
        spin_unlock(&cfg_ptr->lock); \
    } \
    return count; \
} \
static struct kobj_attribute _name##_attr = __ATTR(_name, 0664, _name##_show, _name##_store);

VOIP_ATTR_RW(sip_port)
VOIP_ATTR_RW(rtp_port_start)
VOIP_ATTR_RW(rtp_port_end)
VOIP_ATTR_RW(congestion_thresh)
VOIP_ATTR_RW(priority_mark)

static struct attribute *voip_attrs[] = {
    &sip_port_attr.attr,
    &rtp_port_start_attr.attr,
    &rtp_port_end_attr.attr,
    &congestion_thresh_attr.attr,
    &priority_mark_attr.attr,
    NULL,
};
static struct attribute_group voip_attr_group = {
    .attrs = voip_attrs,
};

int voip_prio_sysfs_init(struct voip_prio_config *cfg)
{
    int ret;
    cfg_ptr = cfg;
    voip_kobj = kobject_create_and_add(VOIP_PRIO_SYSFS_DIR, kernel_kobj);
    if (!voip_kobj)
        return -ENOMEM;
    ret = sysfs_create_group(voip_kobj, &voip_attr_group);
    if (ret)
        kobject_put(voip_kobj);
    return ret;
}

void voip_prio_sysfs_exit(void)
{
    if (voip_kobj) {
        sysfs_remove_group(voip_kobj, &voip_attr_group);
        kobject_put(voip_kobj);
    }
} 