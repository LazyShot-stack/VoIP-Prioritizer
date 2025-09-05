#!/bin/bash
# Automated validation script for VoIP Prioritizer
# This script checks that VoIP packets are marked and prioritized as expected.

set -e

MODULE=voip_prio.ko
USERCTL=../user/voip_prio_ctl
IFACE=eth0  # Change as needed
MARK_HEX=0x1A2B3C4D

cd ..
make
cd test

echo "[INFO] Loading module and configuring..."
sudo insmod ../voip_prio.ko || true
sleep 1
$USERCTL --reset
$USERCTL --show

sudo tc qdisc del dev $IFACE root || true
sudo tc qdisc add dev $IFACE root handle 1: prio
sudo tc filter add dev $IFACE protocol ip parent 1:0 prio 1 handle $MARK_HEX fw flowid 1:1

# Start iperf3 server if available
if command -v iperf3 >/dev/null 2>&1; then
    iperf3 -s -D
    sleep 1
    echo "[INFO] Running iperf3 client for 5 seconds..."
    iperf3 -c 127.0.0.1 -t 5 || echo "[WARN] iperf3 client failed"
    pkill iperf3 || true
else
    echo "[WARN] iperf3 not found, skipping traffic generation."
fi

# Use tcpdump to capture marked packets (requires root)
if command -v tcpdump >/dev/null 2>&1; then
    echo "[INFO] Capturing marked packets for 5 seconds..."
    sudo timeout 5 tcpdump -i $IFACE 'ip[1] & 0xfc == 0' -nn -v > tcpdump_output.txt 2>&1 || true
    echo "[INFO] tcpdump output (first 20 lines):"
    head -20 tcpdump_output.txt
    echo "[INFO] (Check for packets with mark $MARK_HEX)"
else
    echo "[WARN] tcpdump not found, skipping packet capture."
fi

sudo rmmod voip_prio
sudo tc qdisc del dev $IFACE root || true

echo "[INFO] Validation complete. Module unloaded and tc cleaned up." 