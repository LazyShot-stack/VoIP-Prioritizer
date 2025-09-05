# Kernel-Level Traffic Prioritizer for VoIP

## Overview
This project provides a Linux kernel module (`voip_prio.ko`) that identifies and prioritizes VoIP packets (RTP/SIP) in the network stack, ensuring minimal latency and packet loss for real-time voice traffic under network congestion.

## Features
- **Packet Classification:** Identifies VoIP packets by protocol and port (RTP/SIP).
- **Priority Queuing:** Marks VoIP packets for high-priority handling.
- **Dynamic Traffic Shaping:** (Stub) Designed for future bandwidth allocation control.
- **Congestion Detection:** Monitors network device queue length and only prioritizes VoIP when congestion is detected.
- **Token-Bucket Rate Limiting:** Ensures fair bandwidth allocation for VoIP under congestion.
- **User-Space Interface:** Configure via sysfs and a user-space tool.
- **Live Monitoring:** User tool can monitor config and congestion status in real time.
- **Compatibility:** Works with common VoIP protocols and applications.

## File Structure
- `src/` - Kernel module source code
- `user/voip_prio_ctl.c` - User-space configuration tool
- `Makefile` - Build file for kernel module
- `test/` - Test plan and validation scripts

## Build Instructions
1. Ensure you have kernel headers and build tools installed.
2. Build the kernel module:
   ```sh
   make
   ```
3. Build the user-space tool:
   ```sh
   gcc -o voip_prio_ctl user/voip_prio_ctl.c
   ```

## Usage
### Loading the Module
```sh
sudo insmod voip_prio.ko
```

### Configuring via User-Space Tool
```sh
sudo ./voip_prio_ctl --show
sudo ./voip_prio_ctl --sip-port 5060
sudo ./voip_prio_ctl --rtp-port-start 16384
sudo ./voip_prio_ctl --rtp-port-end 32767
sudo ./voip_prio_ctl --congestion-thresh 100
sudo ./voip_prio_ctl --priority-mark 439041101   # 0x1A2B3C4D
sudo ./voip_prio_ctl --reset                     # Reset all to defaults
sudo ./voip_prio_ctl --monitor                   # Live monitor config
```

### Setting Up Traffic Control (tc)
To prioritize marked packets, use `tc` to map the skb->mark to a high-priority qdisc. Example:
```sh
# Example: Map mark to prio qdisc (replace eth0 with your interface)
sudo tc qdisc add dev eth0 root handle 1: prio
sudo tc filter add dev eth0 protocol ip parent 1:0 prio 1 handle 0x1A2B3C4D fw flowid 1:1
```

## Advanced Usage
- **Congestion Detection:** VoIP packets are only prioritized when the device queue exceeds the configured threshold.
- **Token-Bucket:** Prevents VoIP from starving other traffic under heavy load.
- **Live Monitoring:** Use `--monitor` to watch config and congestion status.
- **Custom Priority Mark:** Change the skb->mark value to match your tc setup.

## Troubleshooting
- If sysfs attributes are missing, ensure the module is loaded and sysfs is mounted.
- If packets are not prioritized, check tc filter and mark value.
- Use `dmesg` for kernel logs and debugging.

## Testing
- Use VoIP clients (e.g., Linphone) and background traffic (e.g., iperf).
- Simulate congestion with `netem`:
  ```sh
  sudo tc qdisc add dev eth0 root netem delay 100ms loss 5%
  ```
- Use Wireshark to verify packet marking and prioritization.

## Extending
- Implement a custom qdisc for advanced traffic shaping.
- Add support for encrypted VoIP (SRTP) or other real-time protocols.
- Integrate congestion monitoring and adaptive prioritization.
- Port to eBPF for more flexible packet processing.

## Flow
# Kernel-Level VoIP Traffic Prioritizer Flowchart
[Start: Packet Arrives at Network Interface]
|
v
[Netfilter Hook (NF_INET_POST_ROUTING)]
|  (voip_prio_nf.c)
v
[Packet Inspection]
| - Check IP protocol (UDP/TCP)
| - Extract source/destination ports
| - Compare with configured SIP/RTP ports
v
[Is VoIP Packet?] ----> No ----> [Pass Packet Unmodified (NF_ACCEPT)]
| Yes
v
[Mark Packet with Priority (skb->mark = 0x1A2B3C4D)]
|  (voip_prio_nf.c)
v
[Route to Traffic Control (tc) Subsystem]
| - Custom qdisc or prio qdisc processes marked packets
|  (voip_prio_qdisc.c, currently uses tc)
v
[Prioritize VoIP Packet]
| - High-priority queue for marked packets
| - Low-priority for others
v
[Transmit Packet via Network Interface]
|
v
[Congestion Monitoring (Future)]
| - Check queue lengths or drop rates
| - Adjust prioritization if needed
v
[User-Space Configuration]
| - Sysfs interface (/sys/kernel/voip_prio)
| - User tool (voip_prio_ctl) updates SIP/RTP ports, thresholds
|  (voip_prio_sysfs.c, voip_prio_ctl.c)
v
[Update Kernel Config]
| - Spinlock-protected updates to voip_prio_config
|  (voip_prio.h, voip_prio.c)
v
[End: Loop for Next Packet]

## License

GPL v2 

