/*
 * voip_prio_ctl.c - User-space tool to configure VoIP Prioritizer kernel module via sysfs
 * Usage:
 *   voip_prio_ctl <option> <value>
 * Options:
 *   --sip-port <port>         Set SIP port (default: 5060)
 *   --rtp-port-start <port>   Set RTP port range start
 *   --rtp-port-end <port>     Set RTP port range end
 *   --congestion-thresh <n>   Set congestion threshold
 *   --priority-mark <hex>     Set skb->mark value (e.g., 0x1A2B3C4D)
 *   --reset                   Reset all parameters to defaults
 *   --monitor                 Monitor config values in real time
 *   --show                    Show current settings
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SYSFS_BASE "/sys/kernel/voip_prio/"

const struct {
    const char *name;
    const char *default_val;
} attrs[] = {
    {"sip_port", "5060"},
    {"rtp_port_start", "16384"},
    {"rtp_port_end", "32767"},
    {"congestion_thresh", "100"},
    {"priority_mark", "439041101"}, // 0x1A2B3C4D
};

void show_current() {
    char path[128], buf[64];
    FILE *f;
    for (int i = 0; i < sizeof(attrs)/sizeof(attrs[0]); ++i) {
        snprintf(path, sizeof(path), SYSFS_BASE "%s", attrs[i].name);
        f = fopen(path, "r");
        if (f) {
            if (fgets(buf, sizeof(buf), f))
                printf("%s: %s", attrs[i].name, buf);
            fclose(f);
        } else {
            printf("%s: (not available)\n", attrs[i].name);
        }
    }
}

int set_attr(const char *attr, const char *value) {
    char path[128];
    FILE *f;
    snprintf(path, sizeof(path), SYSFS_BASE "%s", attr);
    f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fprintf(f, "%s\n", value);
    fclose(f);
    return 0;
}

void reset_defaults() {
    for (int i = 0; i < sizeof(attrs)/sizeof(attrs[0]); ++i) {
        set_attr(attrs[i].name, attrs[i].default_val);
    }
    printf("All parameters reset to defaults.\n");
}

void monitor_config() {
    while (1) {
        show_current();
        printf("-----------------------------\n");
        sleep(2);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <option> <value>\n", argv[0]);
        printf("Options:\n");
        printf("  --sip-port <port>\n");
        printf("  --rtp-port-start <port>\n");
        printf("  --rtp-port-end <port>\n");
        printf("  --congestion-thresh <n>\n");
        printf("  --priority-mark <hex>\n");
        printf("  --reset\n");
        printf("  --monitor\n");
        printf("  --show\n");
        return 1;
    }
    if (strcmp(argv[1], "--show") == 0) {
        show_current();
        return 0;
    }
    if (strcmp(argv[1], "--reset") == 0) {
        reset_defaults();
        return 0;
    }
    if (strcmp(argv[1], "--monitor") == 0) {
        monitor_config();
        return 0;
    }
    if (argc < 3) {
        fprintf(stderr, "Missing value for option %s\n", argv[1]);
        return 1;
    }
    if (strcmp(argv[1], "--sip-port") == 0)
        return set_attr("sip_port", argv[2]);
    if (strcmp(argv[1], "--rtp-port-start") == 0)
        return set_attr("rtp_port_start", argv[2]);
    if (strcmp(argv[1], "--rtp-port-end") == 0)
        return set_attr("rtp_port_end", argv[2]);
    if (strcmp(argv[1], "--congestion-thresh") == 0)
        return set_attr("congestion_thresh", argv[2]);
    if (strcmp(argv[1], "--priority-mark") == 0)
        return set_attr("priority_mark", argv[2]);
    fprintf(stderr, "Unknown option: %s\n", argv[1]);
    return 1;
} 