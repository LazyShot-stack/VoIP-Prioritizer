# Automated Test Scripts for VoIP Prioritizer

This directory contains scripts to automate building, loading, configuring, testing, and validating the VoIP prioritizer kernel module.

## Scripts

### automated_test.sh
- **Purpose:**
  - Build the kernel module and user tool
  - Load and configure the module
  - Set up traffic control (tc)
  - Optionally run iperf3 for background traffic
  - Monitor sysfs config
  - Unload and clean up
- **Usage:**
  ```sh
  cd test
  bash automated_test.sh
  ```
- **Requirements:**
  - sudo/root access
  - iperf3 (optional, for traffic generation)
  - tc (traffic control)

### automated_validation.sh
- **Purpose:**
  - Build, load, and configure the module
  - Set up tc
  - Optionally run iperf3 for background traffic
  - Use tcpdump to capture and display marked packets
  - Unload and clean up
- **Usage:**
  ```sh
  cd test
  bash automated_validation.sh
  ```
- **Requirements:**
  - sudo/root access
  - iperf3 (optional)
  - tc
  - tcpdump (optional, for packet capture)

## Troubleshooting
- If you see errors about missing sysfs attributes, ensure the module is loaded and sysfs is mounted.
- If packets are not marked, check the tc filter and mark value.
- Use `dmesg` for kernel logs and debugging.
- Change the `IFACE` variable in the scripts to match your network interface.

## Notes
- These scripts are for development and validation. For production, review and adapt as needed. 