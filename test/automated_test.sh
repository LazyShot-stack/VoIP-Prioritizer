#!/bin/bash
# Automated test script for Kernel-Level Traffic Prioritizer for VoIP
# This script builds, loads, configures, tests, and unloads the kernel module.

set -e

MODULE=voip_prio.ko
USERCTL=../user/voip_prio_ctl
IFACE=eth0  # Change as needed

# 1. Build kernel module and user tool
cd ..
echo "[INFO] Building kernel module..."
make

echo "[INFO] Building user-space control tool..."
gcc -o user/voip_prio_ctl user/voip_prio_ctl.c
cd test

# 2. Load the kernel module
if lsmod | grep -q voip_prio; then
    echo "[INFO] Module already loaded, removing first..."
    sudo rmmod voip_prio
fi

echo "[INFO] Inserting kernel module..."
sudo insmod ../voip_prio.ko || { echo "[ERROR] Failed to insert module"; exit 1; }
sleep 1

# 3. Configure default parameters
$USERCTL --reset
$USERCTL --show

# 4. Set up traffic control (tc) for prioritization
sudo tc qdisc del dev $IFACE root || true
sudo tc qdisc add dev $IFACE root handle 1: prio
sudo tc filter add dev $IFACE protocol ip parent 1:0 prio 1 handle 0x1A2B3C4D fw flowid 1:1

echo "[INFO] Traffic control (tc) set up on $IFACE."

# 5. Start background traffic (iperf, if available)
if command -v iperf3 >/dev/null 2>&1; then
    echo "[INFO] Starting iperf3 server in background..."
    iperf3 -s -D
    sleep 1
    echo "[INFO] Running iperf3 client for 5 seconds..."
    iperf3 -c 127.0.0.1 -t 5 || echo "[WARN] iperf3 client failed"
    pkill iperf3 || true
else
    echo "[WARN] iperf3 not found, skipping traffic generation."
fi

# 6. Monitor sysfs config (show current settings)
echo "[INFO] Monitoring config for 5 seconds..."
$USERCTL --monitor &
MON_PID=$!
sleep 5
kill $MON_PID

# 7. Unload the kernel module
sudo rmmod voip_prio
sudo tc qdisc del dev $IFACE root || true

echo "[INFO] Test complete. Module unloaded and tc cleaned up." 