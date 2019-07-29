
add_library(BaseLib STATIC  
  SwapBytes32.c
  SwapBytes16.c
  LongJump.c
  SetJump.c
  RShiftU64.c
  MultU64x32.c
  LShiftU64.c
  DivU64x32Remainder.c
  DivU64x32.c
  BitField.c
  CpuDeadLoop.c
  Cpu.c
  String.c

  #X64/CpuPause.asm
  #X64/EnableDisableInterrupts.asm
  #X64/DisableInterrupts.asm
  #X64/EnableInterrupts.asm
  #X64/FlushCacheLine.asm
  #X64/Invd.asm
  #X64/Wbinvd.asm
  #X64/ReadTsc.asm
  #X64/FxRestore.asm
  #X64/FxSave.asm
  #X64/WriteLdtr.asm
  #X64/ReadLdtr.asm
  #X64/WriteIdtr.asm
  #X64/ReadIdtr.asm
  #X64/WriteGdtr.asm
  #X64/ReadGdtr.asm
  #X64/ReadTr.asm
  #X64/ReadSs.asm
  #X64/ReadGs.asm
  #X64/ReadFs.asm
  #X64/ReadEs.asm
  #X64/ReadDs.asm
  #X64/ReadCs.asm
  #X64/WriteDr7.asm
  #X64/WriteDr6.asm
  #X64/WriteDr5.asm
  #X64/WriteDr4.asm
  #X64/WriteDr3.asm
  #X64/WriteDr2.asm
  #X64/WriteDr1.asm
  #X64/WriteDr0.asm
  #X64/ReadDr7.asm
  #X64/ReadDr6.asm
  #X64/ReadDr5.asm
  #X64/ReadDr4.asm
  #X64/ReadDr3.asm
  #X64/ReadDr2.asm
  #X64/ReadDr1.asm
  #X64/ReadDr0.asm
  #X64/WriteCr4.asm
  #X64/WriteCr3.asm
  #X64/WriteCr2.asm
  #X64/WriteCr0.asm
  #X64/ReadCr4.asm
  #X64/ReadCr3.asm
  #X64/ReadCr2.asm
  #X64/ReadCr0.asm
  #X64/ReadEflags.asm
  #X64/CpuIdEx.asm
  #X64/CpuId.asm
  #X64/CpuId.S
  #X64/CpuId.s
  #X64/CpuIdEx.s
  # X64/LongJump.asm
  #X64/LongJump.s
  # X64/SetJump.asm
  #X64/SetJump.s
  #X64/EnableCache.asm
  #X64/DisableCache.asm

  #X64/CpuBreakpoint.asm  
  #X64/WriteMsr64.asm  
  #X64/ReadMsr64.asm  

  X64/Non-existing.c
  Math64.c
  Unaligned.c
  X86WriteIdtr.c
  X86WriteGdtr.c
  X86ReadIdtr.c
  X86ReadGdtr.c
  X86Msr.c
  X86MemoryFence.c 
  X86GetInterruptState.c
  X86FxSave.c
  X86FxRestore.c
  X64/GccInline.c 
  X64/SetJump.S 
  X64/LongJump.S  
  #X64/EnableDisableInterrupts.S  
  X64/CpuId.S  
  X64/CpuIdEx.S  
  #X64/EnableCache.S 
  #X64/DisableCache.S
)

target_link_libraries(BaseLib PUBLIC
  PcdLib
  DebugLib
  BaseMemoryLib
)

target_include_directories(BaseLib PRIVATE
	.
)
