module gpsDeploy {

  # ----------------------------------------------------------------------
  # Base ID Convention
  # ----------------------------------------------------------------------
  #
  # All Base IDs follow the 8-digit hex format: 0xDSSCCxxx
  #
  # Where:
  #   D   = Deployment digit (1 for this deployment)
  #   SS  = Subtopology digits (00 for main topology, 01-05 for subtopologies)
  #   CC  = Component digits (00, 01, 02, etc.)
  #   xxx = Reserved for internal component items (events, commands, telemetry)
  #

  # ----------------------------------------------------------------------
  # Defaults
  # ----------------------------------------------------------------------

  module Default {
    constant QUEUE_SIZE = 10
    constant STACK_SIZE = 64 * 1024
  }

  # ----------------------------------------------------------------------
  # Active component instances
  # ----------------------------------------------------------------------

  instance rateGroup1: Svc.ActiveRateGroup base id 0x10001000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 120

  instance rateGroup2: Svc.ActiveRateGroup base id 0x10002000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 119

  instance rateGroup3: Svc.ActiveRateGroup base id 0x10003000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 118

  instance cmdSeq: Svc.CmdSequencer base id 0x10004000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 117

  @ GPS component instance
  instance gps: Gps.Gps base id 0x10005000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 95

  # ----------------------------------------------------------------------
  # Queued component instances
  # ----------------------------------------------------------------------


  # ----------------------------------------------------------------------
  # Passive component instances
  # ----------------------------------------------------------------------

  instance chronoTime: Svc.ChronoTime base id 0x10010000

  instance rateGroupDriver: Svc.RateGroupDriver base id 0x10011000

  instance systemResources: Svc.SystemResources base id 0x10012000

  instance timer: Svc.LinuxTimer base id 0x10013000

  instance comDriver: Drv.TcpClient base id 0x10014000

  instance bufferManager: Svc.BufferManager base id 0x10015000 \
  {
    phase Fpp.ToCpp.Phases.configComponents """
    Fw::MallocAllocator m_allocator;
    Svc::BufferManager::BufferBins bufferManagerBins;
    memset(&bufferManagerBins, 0, sizeof(bufferManagerBins));
    {
      bufferManagerBins.bins[0].bufferSize = 1024;
      bufferManagerBins.bins[0].numBuffers = 100;
      bufferManager.setup(
          200,
          0,
          m_allocator,
          bufferManagerBins
      );
    }
    """
  }

  @ UART driver instance. Configured to use a Linux UART driver
  instance uartDrv: Drv.LinuxUartDriver base id 0x10016000 \
  {
    phase Fpp.ToCpp.Phases.configComponents """
      const bool status = uartDrv.open("/dev/ttyACM0",
          Drv::LinuxUartDriver::BAUD_9600,
          Drv::LinuxUartDriver::NO_FLOW,
          Drv::LinuxUartDriver::PARITY_NONE,
          1024
      );
      if (status) 
      {
        printf("Successfully opened UART driver\\n");
        uartDrv.start();
      } else 
      {
        printf("[ERROR]: Could not open UART driver\\n");
      }
    """

    phase Fpp.ToCpp.Phases.stopTasks """
    uartDrv.quitReadThread();
    """
  }

}
