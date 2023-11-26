#include "cpu.h"
#include "lapic.h"
#include <stdint.h>

#define LAPIC_BASE_ADDR 0xfee00000 // APIC base address

// enabling x2APIC by setting the bit 10 of IA32_APIC_BASE_MSR
// #define IA32_APIC_BASE_MSR 0x1B | (1 << 10) // APIC base MSR
#define IA32_APIC_BASE_MSR 0x1B
// above address might be invalid

// Local APIC registers(offsets)
#define LAPIC_REG_ID 0x20 // LAPIC ID
#define LAPIC_REG_EOI 0xb0 // End of interrupt
#define LAPIC_REG_SPURIOUS 0xf0
#define LAPIC_REG_CMCI 0x2f0 // LVT Corrected machine check interrupt
#define LAPIC_REG_ICR0 0x300 // Interrupt command register
#define LAPIC_REG_ICR1 0x310 // /ICR1 should be written first and then ICR0
#define LAPIC_REG_LVT_TIMER 0x320
#define LAPIC_REG_TIMER_INITCNT 0x380 // Initial count register
#define LAPIC_REG_TIMER_CURCNT 0x390 // Current count register
#define LAPIC_REG_TIMER_DIV 0x3e0
#define LAPIC_EOI_ACK 0x00

// x2APIC registers(offsets)
// #define X2LAPIC_REG 0x800 // x2APIC register
// #define X2LAPIC_REG_ID 0x802 // x2APIC ID
// #define X2LAPIC_REG_EOI 0x80b // x2APIC EOI
// #define X2LAPIC_REG_SPURIOUS 0x80f
// #define X2LAPIC_REG_CMCI 0x82f // x2APIC LVT Corrected machine check interrupt
// #define X2LAPIC_REG_ICR0 0x830 // x2APIC Interrupt command register
// #define X2LAPIC_REG_ICR1 0x831
// #define X2LAPIC_REG_LVT_TIMER 0x832
// #define X2LAPIC_REG_TIMER_INITCNT 0x838 // x2APIC Initial count register
// #define X2LAPIC_REG_TIMER_CURCNT 0x839 // x2APIC Current count register
// #define X2LAPIC_REG_TIMER_DIV 0x83e

// I think this doesn't work...
// But this must be how we send interrupts
void lapic_write(uintptr_t offset, uint32_t value) {
    *(uintptr_t*)(LAPIC_BASE_ADDR + offset) = value;
}

uint32_t lapic_read(uintptr_t offset) {
    return *(uintptr_t*)(LAPIC_BASE_ADDR + offset);
}

void lapic_eoi() {
    lapic_write(LAPIC_REG_EOI, LAPIC_EOI_ACK);
    // be sure to send EOI before iret
}

void lapic_send_ipi(uint32_t id, uint8_t vector) { // IPI: we can send an interrupt to the target
                                                   // core(id) with the given offset(index) 
                                                   // in the interrupt vector(vector)
    lapic_write(LAPIC_REG_ICR1, id << 24);
    lapic_write(LAPIC_REG_ICR0, vector); // by writing to the lower half, we send the interrupt
}

void lapic_timer_stop() {
    lapic_write(LAPIC_REG_TIMER_INITCNT, 0);
    lapic_write(LAPIC_REG_LVT_TIMER, 1 << 16);
}

void lapic_one_shot_timer(uint64_t ticks) {
    lapic_write(LAPIC_REG_TIMER_INITCNT, ticks);
    // lapic_write(LAPIC_REG_LVT_TIMER, 32 | (1 << 17));
}

// NOTE: IDK what the return value is...
uint64_t lapic_reg_id() {
    return lapic_read(LAPIC_REG_ID);
}

// *** send EOI interrupt before iret whilst handling interrupts ***
void lapic_init() {
        
    // enabling apic
    lapic_write(LAPIC_REG_SPURIOUS, 0x1ff | (1 << 8)); // enable spurious interrupt vector
}
