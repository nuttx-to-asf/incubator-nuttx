/****************************************************************************
 * arch/xtensa/src/common/xtensa_exit.c
 *
 *   Copyright (C) 2016-2018 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sched.h>
#include <syscall.h>
#include <assert.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#ifdef CONFIG_DUMP_ON_EXIT
#  include <nuttx/fs/fs.h>
#endif

#include "task/task.h"
#include "sched/sched.h"
#include "group/group.h"
#include "xtensa.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_DEBUG_SCHED_INFO
#  undef CONFIG_DUMP_ON_EXIT
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: _xtensa_dumponexit
 *
 * Description:
 *   Dump the state of all tasks whenever on task exits.  This is debug
 *   instrumentation that was added to check file-related reference counting
 *   but could be useful again sometime in the future.
 *
 ****************************************************************************/

#ifdef CONFIG_DUMP_ON_EXIT
static void _xtensa_dumponexit(FAR struct tcb_s *tcb, FAR void *arg)
{
  FAR struct filelist *filelist;
#ifdef CONFIG_FILE_STREAM
  FAR struct file_struct *filep;
#endif
  int i;
  int j;

  sinfo("  TCB=%p name=%s pid=%d\n", tcb, tcb->argv[0], tcb->pid);
  sinfo("    priority=%d state=%d\n", tcb->sched_priority, tcb->task_state);

  filelist = tcb->group->tg_filelist;
  for (i = 0; i < filelist->fl_rows; i++)
    {
      for (j = 0; j < CONFIG_NFILE_DESCRIPTORS_PER_BLOCK; j++)
        {
          struct inode *inode = filelist->fl_files[i][j].f_inode;
          if (inode)
            {
              sinfo("      fd=%d refcount=%d\n",
                    i * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK + j,
                    inode->i_crefs);
            }
        }
    }

#ifdef CONFIG_FILE_STREAM
  filep = tcb->group->tg_streamlist->sl_head;
  for (; filep != NULL; filep = filep->fs_next)
    {
      if (filep->fs_fd >= 0)
        {
#ifndef CONFIG_STDIO_DISABLE_BUFFERING
          if (filep->fs_bufstart != NULL)
            {
              sinfo("      fd=%d nbytes=%d\n",
                    filep->fs_fd,
                    filep->fs_bufpos - filep->fs_bufstart);
            }
          else
#endif
            {
              sinfo("      fd=%d\n", filep->fs_fd);
            }
        }
    }
#endif
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_exit
 *
 * Description:
 *   This function causes the currently executing task to cease
 *   to exist.  This is a special case of task_delete() where the task to
 *   be deleted is the currently executing task.  It is more complex because
 *   a context switch must be performed to the next ready to run task.
 *
 ****************************************************************************/

void up_exit(int status)
{
  struct tcb_s *tcb = this_task();

  /* Make sure that we are in a critical section with local interrupts.
   * The IRQ state will be restored when the next task is started.
   */

  enter_critical_section();

  sinfo("TCB=%p exiting\n", tcb);

#ifdef CONFIG_DUMP_ON_EXIT
  sinfo("Other tasks:\n");
  nxsched_foreach(_xtensa_dumponexit, NULL);
#endif

#if XCHAL_CP_NUM > 0
  /* Disable co-processor support for the task that is exit-ing. */

  xtensa_coproc_disable(&tcb->xcp.cpstate, XTENSA_CP_ALLSET);
#endif

  /* Destroy the task at the head of the ready to run list. */

  nxtask_exit();

  /* Now, perform the context switch to the new ready-to-run task at the
   * head of the list.
   */

  tcb = this_task();

  /* Adjusts time slice for SCHED_RR & SCHED_SPORADIC cases
   * NOTE: the API also adjusts the global IRQ control for SMP
   */

  nxsched_resume_scheduler(tcb);

#if XCHAL_CP_NUM > 0
  /* Set up the co-processor state for the newly started thread. */

  xtensa_coproc_restorestate(&tcb->xcp.cpstate);
#endif

#ifdef CONFIG_ARCH_ADDRENV
  /* Make sure that the address environment for the previously running
   * task is closed down gracefully (data caches dump, MMU flushed) and
   * set up the address environment for the new thread at the head of
   * the ready-to-run list.
   */

  group_addrenv(tcb);
#endif

  /* Then switch contexts */

  xtensa_context_restore(tcb->xcp.regs);

  /* xtensa_full_context_restore() should not return but could if the
   * software interrupts are disabled.
   */

  DEBUGPANIC();
}
