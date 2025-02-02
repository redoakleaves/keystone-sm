#ifndef _WAYMASKS_H_
#define _WAYMASKS_H_

#include <stdint.h>
#include <stddef.h>

#define WM_NUM_MASTERS 21
#define WM_NUM_WAYS 16
#define WM_NUM_HARTS 4

#define WM_MIN_FREE_WAY WM_NUM_HARTS
#define WM_MAX_FREE_WAY WM_NUM_WAYS-2 /* We allocate 1 way for all non hart masters */

/* Waymasking is part of the dynamic way-allocation system.
 *
 * A Way is in one of 3 global logical states (W_n):
 *  - Free (in use on any master using untrusted code) - F
 *  - Free-reserved (Free, but may not be allocated in all circumstances) - FR
 *  - Allocated (in use for one specific enclave) - A
 *
 * A way is in one of 2 states on each master (MW_n):
 *  - Granted (1 in mask, available to be used) - G
 *  - Locked (0 in mask, not available to be used) - L
 *
 */


/* Legal allocations
 ********************
 * Way  0    : (FR) All non-hart masters, all untrusted execution
 * Ways 1-C  : (FR/A) Hart N MAY always has way N, untrusted or enclave mode.
 *                    Enclave mode is not required to use Way N.
 * Ways C-M  : (F/A) Free or allocated to an enclave
 *
 * C = Number of harts participating
 * M = Total ways
 */

typedef uint64_t waymask_t;

/* Ways currently allocated to enclaves */
extern waymask_t enclave_allocated_ways;

/* Ways currently allocated to the scratchpad */
extern waymask_t scratchpad_allocated_ways;

/* All allocated ways, should be OR of above two */
extern waymask_t allocated_ways;

/* PMP Region ID for the scratchpad */
extern region_id scratch_rid;
extern region_id l2_controller_rid;

// Waymask master IDs
#define WM_Hart_0_DCache_MMIO           0
#define WM_Hart_1_DCache                2
#define WM_Hart_2_DCache                4
#define WM_Hart_3_DCache                6
#define WM_Hart_4_DCache                8
#define WM_DMA                          10
#define WM_GEMGXL                       11


// Cache controller is mapped to this address for the FU740
#define CACHE_CONTROLLER_ADDR_START (uintptr_t)0x02010000
#define CACHE_CONTROLLER_ADDR_END  0x02011000 // Not inclusive
// WM Registers are 8 bytes apart, starting at 0x800
#define WM_REG_OFFSET(id) (uintptr_t)(0x800+(8*id))
#define WM_REG_ADDR(id) ((waymask_t*)( CACHE_CONTROLLER_ADDR_START + WM_REG_OFFSET(id)))

// Only the lower 16 bits are used
#define WM_MASK 0xFFFF

// Invert a given mask
#define WM_FLIP_MASK(mask) ((~mask) & WM_MASK)

// 64-bit address flush register
#define L2_FLUSH64_REG ((uintptr_t*)(CACHE_CONTROLLER_ADDR_START + 0x200))

#define IS_WAY_ALLOCATED( waynum ) ( allocated_ways & (0x1 << waynum))

#define IS_MASTER_RUNNING_UNTRUSTED( master ) (/* TODO */ 0)

//NOTE: We ignore hart1, its a special hart and is disabled. There is
//no hart0?
#define GET_HART_WAY( hartnum ) ( hartnum - 2 )

#define GET_CORE_DWAY( core ) ( core * 2 )

// L2 Zero Device (Scratchpad) info
#define L2_SCRATCH_START (0x0A000000)
#define L2_SCRATCH_STOP  (0x0C000000)

/* 2 MB */
#define L2_SIZE (2*1024*1024)

// 512 sets per bank, 4 banks
#define L2_NUM_SETS 512*4
#define L2_SET_SIZE (L2_SIZE/L2_NUM_SETS)

// Used for LIM/Zero Device
#define L2_WAY_SIZE (L2_SIZE/WM_NUM_WAYS)
#define L2_LINE_SIZE (64)

/* Interface */
size_t waymask_allocate_ways(size_t n_ways, unsigned int target_hart,
                             waymask_t* mask);
void waymask_apply_allocated_mask(waymask_t mask, unsigned int target_hart);
void waymask_free_ways(waymask_t _mask);
void waymask_init();
void waymask_clear_ways(waymask_t mask, unsigned int core);

void waymask_allocate_scratchpad();
void waymask_free_scratchpad();

/* Internals */
int _wm_choose_ways_for_hart(size_t n_ways, waymask_t* _mask, unsigned int target_hart);
int _wm_lockout_ways(waymask_t mask, unsigned int master);
int _wm_grant_ways(waymask_t mask, unsigned int hart);
int _wm_assign_mask(waymask_t mask, unsigned int master);
void waymask_debug_printstatus();

#endif /* _WAYMASKS_H_ */
