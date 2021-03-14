//******************************************************************************
// Copyright (c) 2018, The Regents of the University of California (Regents).
// All Rights Reserved. See LICENSE for license details.
//------------------------------------------------------------------------------
#include "sm-sbi.h"
#include "pmp.h"
#include "enclave.h"
#include "page.h"
#include "cpu.h"
#include "platform-hook.h"
#include "plugins/plugins.h"
#include "debug.h"
#include <sbi/riscv_asm.h>
#include <sbi/sbi_console.h>

unsigned long sbi_sm_create_enclave(unsigned long* eid, uintptr_t create_args)
{
  struct keystone_sbi_create create_args_local;
  unsigned long ret;

  ret = copy_enclave_create_args(create_args, &create_args_local);

  if (ret)
    return ret;

  ret = create_enclave(eid, create_args_local);

  DEBUG("create_enclave, eid = %lu, ret = %lu", *eid, ret);
  return ret;
}

unsigned long
sbi_clone(unsigned long* eid, uintptr_t create_args){
  unsigned long ret;
  struct keystone_sbi_clone_create create_args_local;

  ret = copy_enclave_clone_args(create_args, &create_args_local);

  if (ret)
    return ret;

  ret = clone_enclave (eid, create_args_local);

  DEBUG("clone_enclave, eid = %lu, ret = %lu", *eid, ret);

  return ret;
}

unsigned long
sbi_snapshot(struct sbi_trap_regs *regs)
{
  unsigned long ret;

  //Returns snapshot handle
  ret = create_snapshot(regs, cpu_get_enclave_id());

  DEBUG("create_snapshot, eid = %d, ret = %lu", cpu_get_enclave_id(), ret);

  return ret;
}


unsigned long sbi_sm_destroy_enclave(unsigned long eid)
{
  unsigned long ret;
  ret = destroy_enclave((unsigned int)eid);

  DEBUG("destroy_enclave, eid = %lu, ret = %lu", eid, ret);

  return ret;
}

unsigned long sbi_sm_run_enclave(struct sbi_trap_regs *regs, unsigned long eid)
{
  regs->a0 = run_enclave(regs, (unsigned int) eid);
  regs->mepc += 4;

  DEBUG("run_enclave, eid = %lu, ret = %lu", eid, regs->a0);

  sbi_trap_exit(regs);
  return 0;
}

unsigned long sbi_sm_resume_enclave(struct sbi_trap_regs *regs, unsigned long eid)
{
  unsigned long ret;
  ret = resume_enclave(regs, (unsigned int) eid);
  if (!regs->zero)
    regs->a0 = ret;
  regs->mepc += 4;

  DEBUG("resume_enclave, eid = %lu, ret = %lu", eid, ret);

  sbi_trap_exit(regs);
  return 0;
}

unsigned long sbi_sm_exit_enclave(struct sbi_trap_regs *regs, unsigned long retval)
{
  regs->a0 = exit_enclave(regs, cpu_get_enclave_id());
  regs->a1 = retval;
  regs->mepc += 4;

  DEBUG("exit_enclave, eid = %d, ret = %lu, retval = %lu", cpu_get_enclave_id(), regs->a0, regs->a1);

  sbi_trap_exit(regs);
  return 0;
}

unsigned long sbi_sm_stop_enclave(struct sbi_trap_regs *regs, unsigned long request)
{
  regs->a0 = stop_enclave(regs, request, cpu_get_enclave_id());
  regs->mepc += 4;

  DEBUG("stop_enclave, eid = %d, ret = %lu, request = %lu", cpu_get_enclave_id(), regs->a0, request);

  sbi_trap_exit(regs);
  return 0;
}

unsigned long sbi_sm_attest_enclave(uintptr_t report, uintptr_t data, uintptr_t size)
{
  unsigned long ret;
  ret = attest_enclave(report, data, size, cpu_get_enclave_id());
  return ret;
}

unsigned long sbi_sm_get_sealing_key(uintptr_t sealing_key, uintptr_t key_ident,
                       size_t key_ident_size)
{
  unsigned long ret;
  ret = get_sealing_key(sealing_key, key_ident, key_ident_size,
                         cpu_get_enclave_id());
  return ret;
}

unsigned long sbi_sm_random()
{
  return (unsigned long) platform_random();
}

unsigned long sbi_sm_call_plugin(uintptr_t plugin_id, uintptr_t call_id, uintptr_t arg0, uintptr_t arg1)
{
  unsigned long ret;
  ret = call_plugin(cpu_get_enclave_id(), plugin_id, call_id, arg0, arg1);
  return ret;
}
