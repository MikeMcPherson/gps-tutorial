// ======================================================================
// \title  Gps.hpp
// \author lestarch and Mike McPherson
// \brief  hpp file for Gps component implementation class
// 
// The F' component implements a GPS receiver that processes NMEA 
// sentences.  It handles GPS data reception, parsing, and provides 
// commands to enable/disable GPS functionality. The component maintains 
// state variables for Gps status, fix validity, and geographic 
// coordinates.  It also provides telemetry outputs for GPS data.
// ======================================================================

#ifndef Gps_Gps_HPP
#define Gps_Gps_HPP

#include "Components/Gps/GpsComponentAc.hpp"

namespace Gps {

  class Gps :
    public GpsComponentBase
  {

      /**
       * GpsPacket:
       *   A structure containing the information in the GPS location packet
       * received via the NMEA GPS receiver.
       */
      struct GpsPacket {
          char constellation;
          float utcTime;
          float dmNS;
          char northSouth;
          float dmEW;
          char eastWest;
          unsigned int lock;
          unsigned int count;
          float filler;
          float altitude;
      } m_packet;

    public:

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Gps object
      Gps(
          const char* const compName //!< The component name
      );

      //! Destroy Gps object
      ~Gps();

    PRIVATE:

      char m_sentenceBuffer[128] = {0}; //!< Buffer for NMEA sentences
      U32 m_sentenceBufferIndex = 0; //!< Pointer to the current position in the sentence buffer
      U32 m_numSentences = 0; //!< Number of sentences received
      U32 m_locked = 0; //!< GGA GPS Quality
      Fw::On m_GpsEnabled = Fw::On::OFF; //!< Gps enabled flag
      Fw::On m_fixValid = Fw::On::OFF; //!< Valid fix flag

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for GpsRecv
      void GpsRecv_handler(
          FwIndexType portNum, //!< The port number
          Fw::Buffer& recvBuffer, //!< The received buffer
          const Drv::RecvStatus& recvStatus //!< The receive status
      ) override;

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------

      //! Handler implementation for command GpsEnable
      //!
      //! Gps enable command
      void GpsEnable_cmdHandler(
          FwOpcodeType opCode, //!< The opcode
          U32 cmdSeq, //!< The command sequence number
          Fw::On newStatus //!< The new status to set
      ) override;

  };

}

#endif
