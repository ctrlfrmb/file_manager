#!/bin/bash
# Network setup and configuration script
# Creates and configures network namespaces, interfaces, IP addresses and routing

# Author: figkey-leiwei

set -e

# Log function
log() {
    echo "[$(date +"%H:%M:%S")] $1"
}

# ============= Namespace Initialization Section =============

log "Starting DUT initialization..."
# Clean existing network configuration
log "Starting to clean existing network configuration..."

# Clean configuration for DUT 1
if ip netns list | grep -q "dut1"; then
    # If namespace has eth interface, move it back to root namespace first
    if ip netns exec dut1 ip link show eth1 &>/dev/null 2>&1; then
        ip netns exec dut1 ip link set eth1 netns 1 2>/dev/null || true
    fi
    # Delete namespace
    ip netns del dut1 2>/dev/null || true
fi
# Delete veth pair if exists
if ip link show veth-host1 &>/dev/null 2>&1; then
    ip link del veth-host1 &>/dev/null || true
fi

# Clean configuration for DUT 2
if ip netns list | grep -q "dut2"; then
    # If namespace has eth interface, move it back to root namespace first
    if ip netns exec dut2 ip link show eth2 &>/dev/null 2>&1; then
        ip netns exec dut2 ip link set eth2 netns 1 2>/dev/null || true
    fi
    # Delete namespace
    ip netns del dut2 2>/dev/null || true
fi
# Delete veth pair if exists
if ip link show veth-host2 &>/dev/null 2>&1; then
    ip link del veth-host2 &>/dev/null || true
fi

# Clean configuration for DUT 3
if ip netns list | grep -q "dut3"; then
    # If namespace has eth interface, move it back to root namespace first
    if ip netns exec dut3 ip link show eth3 &>/dev/null 2>&1; then
        ip netns exec dut3 ip link set eth3 netns 1 2>/dev/null || true
    fi
    # Delete namespace
    ip netns del dut3 2>/dev/null || true
fi
# Delete veth pair if exists
if ip link show veth-host3 &>/dev/null 2>&1; then
    ip link del veth-host3 &>/dev/null || true
fi

# Clean configuration for DUT 4
if ip netns list | grep -q "dut4"; then
    # If namespace has eth interface, move it back to root namespace first
    if ip netns exec dut4 ip link show eth4 &>/dev/null 2>&1; then
        ip netns exec dut4 ip link set eth4 netns 1 2>/dev/null || true
    fi
    # Delete namespace
    ip netns del dut4 2>/dev/null || true
fi
# Delete veth pair if exists
if ip link show veth-host4 &>/dev/null 2>&1; then
    ip link del veth-host4 &>/dev/null || true
fi

# Clean configuration for DUT 5
if ip netns list | grep -q "dut5"; then
    # If namespace has eth interface, move it back to root namespace first
    if ip netns exec dut5 ip link show eth5 &>/dev/null 2>&1; then
        ip netns exec dut5 ip link set eth5 netns 1 2>/dev/null || true
    fi
    # Delete namespace
    ip netns del dut5 2>/dev/null || true
fi
# Delete veth pair if exists
if ip link show veth-host5 &>/dev/null 2>&1; then
    ip link del veth-host5 &>/dev/null || true
fi

# Clean configuration for DUT 6
if ip netns list | grep -q "dut6"; then
    # If namespace has eth interface, move it back to root namespace first
    if ip netns exec dut6 ip link show eth6 &>/dev/null 2>&1; then
        ip netns exec dut6 ip link set eth6 netns 1 2>/dev/null || true
    fi
    # Delete namespace
    ip netns del dut6 2>/dev/null || true
fi
# Delete veth pair if exists
if ip link show veth-host6 &>/dev/null 2>&1; then
    ip link del veth-host6 &>/dev/null || true
fi

# Check if br0 exists
if ! ip link show br0 &>/dev/null 2>&1; then
    log "Creating bridge br0..."
    ip link add name br0 type bridge
    ip link set dev br0 up
fi

# Create veth pairs and connect to bridge
log "Creating veth pairs and connecting to bridge..."

# Create veth pair for DUT 1
ip link add veth-dut1 type veth peer name veth-host1
ip link set veth-host1 master br0
ip link set veth-host1 up

# Create veth pair for DUT 2
ip link add veth-dut2 type veth peer name veth-host2
ip link set veth-host2 master br0
ip link set veth-host2 up

# Create veth pair for DUT 3
ip link add veth-dut3 type veth peer name veth-host3
ip link set veth-host3 master br0
ip link set veth-host3 up

# Create veth pair for DUT 4
ip link add veth-dut4 type veth peer name veth-host4
ip link set veth-host4 master br0
ip link set veth-host4 up

# Create veth pair for DUT 5
ip link add veth-dut5 type veth peer name veth-host5
ip link set veth-host5 master br0
ip link set veth-host5 up

# Create veth pair for DUT 6
ip link add veth-dut6 type veth peer name veth-host6
ip link set veth-host6 master br0
ip link set veth-host6 up

# Create namespaces and configure interfaces
log "Creating network namespaces and configuring interfaces..."

# Configure DUT 1
log "Configuring DUT 1 (namespace dut1)..."
ip netns add dut1
# Move veth interface into namespace
ip link set veth-dut1 netns dut1
# Check and move physical interface if exists
if ip link show eth1 &>/dev/null 2>&1; then
    ip link set eth1 netns dut1 || true
    log "Moved physical interface eth1 to namespace dut1"
else
    log "Physical interface eth1 not found, skipping"
fi

# Configure DUT 2
log "Configuring DUT 2 (namespace dut2)..."
ip netns add dut2
# Move veth interface into namespace
ip link set veth-dut2 netns dut2
# Check and move physical interface if exists
if ip link show eth2 &>/dev/null 2>&1; then
    ip link set eth2 netns dut2 || true
    log "Moved physical interface eth2 to namespace dut2"
else
    log "Physical interface eth2 not found, skipping"
fi

# Configure DUT 3
log "Configuring DUT 3 (namespace dut3)..."
ip netns add dut3
# Move veth interface into namespace
ip link set veth-dut3 netns dut3
# Check and move physical interface if exists
if ip link show eth3 &>/dev/null 2>&1; then
    ip link set eth3 netns dut3 || true
    log "Moved physical interface eth3 to namespace dut3"
else
    log "Physical interface eth3 not found, skipping"
fi

# Configure DUT 4
log "Configuring DUT 4 (namespace dut4)..."
ip netns add dut4
# Move veth interface into namespace
ip link set veth-dut4 netns dut4
# Check and move physical interface if exists
if ip link show eth4 &>/dev/null 2>&1; then
    ip link set eth4 netns dut4 || true
    log "Moved physical interface eth4 to namespace dut4"
else
    log "Physical interface eth4 not found, skipping"
fi

# Configure DUT 5
log "Configuring DUT 5 (namespace dut5)..."
ip netns add dut5
# Move veth interface into namespace
ip link set veth-dut5 netns dut5
# Check and move physical interface if exists
if ip link show eth5 &>/dev/null 2>&1; then
    ip link set eth5 netns dut5 || true
    log "Moved physical interface eth5 to namespace dut5"
else
    log "Physical interface eth5 not found, skipping"
fi

# Configure DUT 6
log "Configuring DUT 6 (namespace dut6)..."
ip netns add dut6
# Move veth interface into namespace
ip link set veth-dut6 netns dut6
# Check and move physical interface if exists
if ip link show eth6 &>/dev/null 2>&1; then
    ip link set eth6 netns dut6 || true
    log "Moved physical interface eth6 to namespace dut6"
else
    log "Physical interface eth6 not found, skipping"
fi

# Show namespace list
log "Setup complete, created the following namespaces:"
ip netns list
log "DUT initialization completed"

# ============= IP Configuration Section =============

log "Starting IP configuration..."
# Ensure 8021q kernel module is loaded for VLAN support
if ! lsmod | grep -q 8021q; then
    log "Loading 8021q kernel module"
    modprobe 8021q || log "WARNING: Failed to load 8021q module, VLAN interfaces may not work"
fi

# Configure IP addresses for DUT 1 (namespace dut1)
ip netns exec dut1 ip link set veth-dut1 up
log "Adding IP 192.168.10.11/24 to virtual interface veth-dut1"
ip netns exec dut1 ip addr add 192.168.10.11/24 dev veth-dut1 2>/dev/null || true
# Check if physical interface exists in namespace
if ip netns exec dut1 ip link show eth1 &>/dev/null; then
    ip netns exec dut1 ip link set eth1 up
    ip netns exec dut1 ip link del eth1.20 2>/dev/null || true
    log "Creating VLAN interface eth1.20 in namespace dut1"
    if ip netns exec dut1 ip link show eth1.20 &>/dev/null 2>&1; then
        log "VLAN interface eth1.20 already exists, reusing it"
    else
        if ! ip netns exec dut1 ip link add link eth1 name eth1.20 type vlan id 20; then
            log "ERROR: Failed to create VLAN interface eth1.20, check kernel module and interface state"
            log "Interface status: $(ip netns exec dut1 ip -d link show eth1 2>&1)"
        fi
    fi
    if ip netns exec dut1 ip link show eth1.20 &>/dev/null 2>&1; then
        ip netns exec dut1 ip link set eth1.20 up
        log "Adding IP 192.168.20.11/24 to VLAN interface eth1.20"
        ip netns exec dut1 ip addr add 192.168.20.11/24 dev eth1.20 2>/dev/null || true
    else
        log "WARNING: Cannot configure IP for eth1.20 as interface creation failed"
    fi
else
    log "Physical interface eth1 not found in namespace dut1, skipping IP configuration"
fi

# Configure IP addresses for DUT 2 (namespace dut2)
ip netns exec dut2 ip link set veth-dut2 up
# Check if physical interface exists in namespace
if ip netns exec dut2 ip link show eth2 &>/dev/null; then
    ip netns exec dut2 ip link set eth2 up
    ip netns exec dut2 ip link del eth2.20 2>/dev/null || true
    log "Creating VLAN interface eth2.20 in namespace dut2"
    if ip netns exec dut2 ip link show eth2.20 &>/dev/null 2>&1; then
        log "VLAN interface eth2.20 already exists, reusing it"
    else
        if ! ip netns exec dut2 ip link add link eth2 name eth2.20 type vlan id 20; then
            log "ERROR: Failed to create VLAN interface eth2.20, check kernel module and interface state"
            log "Interface status: $(ip netns exec dut2 ip -d link show eth2 2>&1)"
        fi
    fi
    if ip netns exec dut2 ip link show eth2.20 &>/dev/null 2>&1; then
        ip netns exec dut2 ip link set eth2.20 up
        log "Adding IP 192.168.20.101/24 to VLAN interface eth2.20"
        ip netns exec dut2 ip addr add 192.168.20.101/24 dev eth2.20 2>/dev/null || true
    else
        log "WARNING: Cannot configure IP for eth2.20 as interface creation failed"
    fi
else
    log "Physical interface eth2 not found in namespace dut2, skipping IP configuration"
fi

# Configure IP addresses for DUT 3 (namespace dut3)
ip netns exec dut3 ip link set veth-dut3 up
# Check if physical interface exists in namespace
if ip netns exec dut3 ip link show eth3 &>/dev/null; then
    ip netns exec dut3 ip link set eth3 up
    log "Adding IP 192.168.20.11/24 to physical interface eth3"
    ip netns exec dut3 ip addr add 192.168.20.11/24 dev eth3 2>/dev/null || true
else
    log "Physical interface eth3 not found in namespace dut3, skipping IP configuration"
fi

# Configure IP addresses for DUT 4 (namespace dut4)
ip netns exec dut4 ip link set veth-dut4 up
# Check if physical interface exists in namespace
if ip netns exec dut4 ip link show eth4 &>/dev/null; then
    ip netns exec dut4 ip link set eth4 up
    log "Adding IP 192.168.20.12/24 to physical interface eth4"
    ip netns exec dut4 ip addr add 192.168.20.12/24 dev eth4 2>/dev/null || true
else
    log "Physical interface eth4 not found in namespace dut4, skipping IP configuration"
fi

# Configure IP addresses for DUT 5 (namespace dut5)
ip netns exec dut5 ip link set veth-dut5 up
# Check if physical interface exists in namespace
if ip netns exec dut5 ip link show eth5 &>/dev/null; then
    ip netns exec dut5 ip link set eth5 up
    log "Adding IP 192.168.20.11/24 to physical interface eth5"
    ip netns exec dut5 ip addr add 192.168.20.11/24 dev eth5 2>/dev/null || true
else
    log "Physical interface eth5 not found in namespace dut5, skipping IP configuration"
fi

# Configure IP addresses for DUT 6 (namespace dut6)
ip netns exec dut6 ip link set veth-dut6 up
# Check if physical interface exists in namespace
if ip netns exec dut6 ip link show eth6 &>/dev/null; then
    ip netns exec dut6 ip link set eth6 up
    log "Adding IP 192.168.20.12/24 to physical interface eth6"
    ip netns exec dut6 ip addr add 192.168.20.12/24 dev eth6 2>/dev/null || true
else
    log "Physical interface eth6 not found in namespace dut6, skipping IP configuration"
fi

log "IP configuration completed"

# ============= Routing Configuration Section =============

log "Starting routing configuration..."
# Configure routing for DUT 1 (namespace dut1)
log "Routing disabled for DUT 1 (namespace dut1), skipping configuration"

# Configure routing for DUT 2 (namespace dut2)
log "Routing disabled for DUT 2 (namespace dut2), skipping configuration"

# Configure routing for DUT 3 (namespace dut3)
log "Routing disabled for DUT 3 (namespace dut3), skipping configuration"

# Configure routing for DUT 4 (namespace dut4)
log "Routing disabled for DUT 4 (namespace dut4), skipping configuration"

# Configure routing for DUT 5 (namespace dut5)
log "Routing disabled for DUT 5 (namespace dut5), skipping configuration"

# Configure routing for DUT 6 (namespace dut6)
log "Routing disabled for DUT 6 (namespace dut6), skipping configuration"

log "Routing configuration completed"

# ============= Network Optimization (IRQ Binding) Section =============

# Author: figkey-leiwei

log "Starting network optimization..."
# ============= Network Interface IRQ CPU Binding Section =============
log "Starting network interface IRQ CPU binding..."

# Check if /proc/interrupts exists
if [ ! -f "/proc/interrupts" ]; then
    log "ERROR: Cannot find /proc/interrupts file, skipping IRQ binding"
    return 1
fi

# Bind network interface IRQs to specific CPUs (eth1->CPU1, eth2->CPU2, etc.)
log "Binding network interfaces eth1-eth6 to CPU1-CPU6..."

# Process each interface from eth1 to eth6
for i in {1..6}; do
    log "Processing eth${i} interface..."
    
    # Get IRQ numbers for this interface
    irqs=$(grep -E "eth${i}" /proc/interrupts | awk '{print $1}' | tr -d :)
    
    if [ -z "$irqs" ]; then
        log "WARNING: No IRQs found for eth${i}, skipping"
        continue
    fi
    
    # Bind each IRQ to corresponding CPU core
    for irq in $irqs; do
        if [ -d "/proc/irq/$irq" ]; then
            echo $i > /proc/irq/$irq/smp_affinity_list 2>/dev/null
            
            if [ $? -eq 0 ]; then
                log "SUCCESS: IRQ $irq (eth${i}) bound to CPU${i}"
            else
                log "ERROR: Failed to bind IRQ $irq (eth${i}) to CPU${i}"
            fi
        else
            log "ERROR: IRQ directory /proc/irq/$irq does not exist"
        fi
    done
done

# Verify IRQ binding results
log "Verifying IRQ binding results..."
grep -E "eth[1-6]" /proc/interrupts | sort
log "IRQ binding completed"
log "Network optimization completed"

log "Script execution completed successfully"
