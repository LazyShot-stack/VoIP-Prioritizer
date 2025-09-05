# Test Plan: Kernel-Level Traffic Prioritizer for VoIP

## Test Environment
- Linux VM or physical machine with kernel module support
- VoIP client/server (e.g., Linphone)
- iperf for background traffic
- netem for network emulation
- Wireshark for packet inspection

## Setup
1. Build and load the kernel module:
   ```sh
   make
   sudo insmod voip_prio.ko
   ```
2. Build the user-space tool:
   ```sh
   gcc -o voip_prio_ctl user/voip_prio_ctl.c
   ```
3. Configure VoIP ports and thresholds as needed:
   ```sh
   sudo ./voip_prio_ctl --sip-port 5060
   sudo ./voip_prio_ctl --rtp-port-start 16384
   sudo ./voip_prio_ctl --rtp-port-end 32767
   sudo ./voip_prio_ctl --congestion-thresh 100
   sudo ./voip_prio_ctl --priority-mark 439041101
   ```
4. Set up traffic control (tc) to prioritize marked packets:
   ```sh
   sudo tc qdisc add dev eth0 root handle 1: prio
   sudo tc filter add dev eth0 protocol ip parent 1:0 prio 1 handle 0x1A2B3C4D fw flowid 1:1
   ```

## Test Cases
### 1. Baseline VoIP Quality (No Congestion)
- Start a VoIP call and measure latency, jitter, and packet loss.
- Expected: Low latency/jitter, minimal packet loss.

### 2. VoIP Quality Under Congestion (Without Module)
- Start background traffic with iperf.
- Start a VoIP call and measure metrics.
- Expected: Increased latency/jitter, higher packet loss.

### 3. VoIP Quality Under Congestion (With Module)
- Load the module and configure as above.
- Start background traffic with iperf.
- Start a VoIP call and measure metrics.
- Expected: Lower latency/jitter and packet loss for VoIP compared to case 2.

### 4. Packet Marking Verification
- Use Wireshark to confirm VoIP packets are marked with 0x1A2B3C4D (or configured mark).
- Expected: All RTP/SIP packets have the correct mark when congestion is present.

### 5. Dynamic Configuration
- Change SIP/RTP ports, congestion threshold, and priority mark via user tool during a call.
- Expected: Module adapts to new settings without reload.

### 6. Token-Bucket Rate Limiting
- Under heavy VoIP load, verify that not all VoIP packets are marked if token-bucket is exhausted.
- Expected: VoIP is prioritized but does not starve other traffic.

### 7. Live Monitoring
- Run `./voip_prio_ctl --monitor` and change parameters in another terminal.
- Expected: Changes are reflected in real time.

## Troubleshooting
- If sysfs attributes are missing, check module load and sysfs mount.
- If packets are not marked, check congestion threshold and tc filter.
- Use `dmesg` for kernel logs and debugging.

## Metrics
- **Latency (ms)**
- **Jitter (ms)**
- **Packet Loss (%)**
- **VoIP MOS (Mean Opinion Score, if available)**

## Expected Results
- VoIP traffic is prioritized and maintains quality under congestion.
- Non-VoIP traffic is fairly handled when not congested.
- Configuration changes take effect immediately.
- Token-bucket prevents VoIP starvation of other traffic. 