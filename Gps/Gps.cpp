// ======================================================================
// \title  Gps.cpp
// \author kq9p
// \brief  cpp file for Gps component implementation class
//
// The F' component implements a GPS receiver that processes NMEA 
// sentences.  It handles GPS data reception, parsing, and provides 
// commands to enable/disable GPS functionality. The component maintains 
// state variables for Gps status, fix validity, and geographic 
// coordinates.  It also provides telemetry outputs for GPS data.
// ======================================================================

#include "Components/Gps/Gps.hpp"
#include "Fw/Logger/Logger.hpp"
#include "Fw/Time/Time.hpp"

namespace Gps {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Gps ::Gps(const char* const compName) : GpsComponentBase(compName) 

{
    m_numSentences = 0; //!< Number of NMEA sentences received
    memset(m_sentenceBuffer, 0, sizeof(m_sentenceBuffer));
}

Gps ::~Gps() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void Gps ::GpsRecv_handler(
    FwIndexType portNum, 
    Fw::Buffer& recvBuffer, 
    const Drv::ByteStreamStatus& recvStatus) 
{
    // Check the receive status
    if(recvStatus == Drv::ByteStreamStatus::RECV_NO_DATA)
    {
      // Handle no data case
      Fw::Logger::log("No data received on port %d\n", portNum);
      this->deallocate_out(0, recvBuffer);// Return the buffer to the deallocate port
      return;
    } else if(recvStatus != Drv::ByteStreamStatus::OP_OK)
    {
      // Handle error case
      Fw::Logger::log("Receive error on port %d, recvStatus: %d\n", portNum, recvStatus);
      this->deallocate_out(0, recvBuffer);// Return the buffer to the deallocate port
      return;
    }

    // Is the Gps enabled?
    if (m_GpsEnabled != Fw::On::ON) {
      this->deallocate_out(0, recvBuffer);// Return the buffer to the deallocate port
      return;
    }

    // Process the NMEA data
    U32 m_bufferSize = recvBuffer.getSize();// How many bytes were received?
    char* m_data_pointer = reinterpret_cast<char*>(recvBuffer.getData());// Get the data pointer from the buffer
    for (U32 i = 0; i < m_bufferSize && i < sizeof(m_sentenceBuffer) - 1; i++) {
      m_sentenceBuffer[m_sentenceBufferIndex] = m_data_pointer[i];// Copy the received data into the sentence buffer
      m_sentenceBufferIndex++;// Increment the pointer to the next position
      // If we have reached the end of an NMEA sentence, process it
      if(m_data_pointer[i] == '\n') {
        m_sentenceBuffer[m_sentenceBufferIndex] = '\0';// Null-terminate the sentence buffer
        m_sentenceBufferIndex++;// Increment the sentence buffer index
        m_numSentences++;// Increment the number of sentences received
        this->tlmWrite_numSentences(m_numSentences);// Update telemetry

        // Parse the GPS message from the UART (looking for $GPGGA messages). This uses standard C functions to read all
        // the defined protocol messages into our GPS package struct.
        U32 status = sscanf(m_sentenceBuffer, "$G%1cGGA,%f,%f,%c,%f,%c,%u,%u,%f,%f",
            &m_packet.constellation,
            &m_packet.utcTime, &m_packet.dmNS, &m_packet.northSouth,
            &m_packet.dmEW, &m_packet.eastWest, &m_packet.lock,
            &m_packet.count, &m_packet.filler, &m_packet.altitude);
        // If we failed to find the G?GGA then move on to the next sentence.
        if (status != 10) {
          // Log an error if parsing failed
          Fw::Logger::log("[ERROR] GPS parsing failed: %d\n", status);
          m_sentenceBufferIndex = 0; //!< Reset the sentence buffer index
          continue;
        }
        //GPS packet locations are of the form: ddmm.mmmm
        //We will convert to lat/lon in degrees only before downlinking
        //Latitude degrees, add on minutes (converted to degrees), multiply by direction
        F32 lat = (U32)(m_packet.dmNS/100.0f);
        lat = lat + (m_packet.dmNS - (lat * 100.0f))/60.0f;
        lat = lat * ((m_packet.northSouth == 'N') ? 1 : -1);
        //Longitude degrees, add on minutes (converted to degrees), multiply by direction
        F32 lon = (U32)(m_packet.dmEW/100.0f);
        lon = lon + (m_packet.dmEW - (lon * 100.0f))/60.f;
        lon = lon * ((m_packet.eastWest == 'E') ? 1 : -1);
        //Step 4: call the downlink functions to send down data
        this->tlmWrite_latitude(lat);
        this->tlmWrite_longitude(lon);
        this->tlmWrite_altitude(m_packet.altitude);
        //Lock status update only if changed
        // Emit an event if the lock has been acquired, or lost
        if (m_packet.lock > 0 && m_packet.lock < 4)
        {
          m_packet.lock = 1;
        } else
        {
          m_packet.lock = 0;
        }
        if (m_packet.lock == 0 && m_locked == Fw::On::ON) {
            m_locked = Fw::On::OFF; // Set GPS lock flag to OFF
            m_fixValid = Fw::On::OFF; // Set fix validity to OFF
            this->log_ACTIVITY_HI_fixValidity(m_fixValid);
        } else if (m_packet.lock == 1 && m_locked == Fw::On::OFF) {
            m_locked = Fw::On::ON; // Set GPS lock flag to ON
            m_fixValid = Fw::On::ON; // Set fix validity to ON
            this->log_ACTIVITY_HI_fixValidity(m_fixValid);
        }
        // Reset the pointer to the start of the sentence buffer
        m_sentenceBufferIndex = 0;
      }
    }
    // Return the buffer to the deallocate port
    this->deallocate_out(0, recvBuffer);
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void Gps ::GpsEnable_cmdHandler(
    FwOpcodeType opCode, 
    U32 cmdSeq, 
    Fw::On newStatus) 
{
    // Save the new Gps enabled status
    m_GpsEnabled = newStatus;
    if(m_GpsEnabled == Fw::On::OFF) {
      // Reset to default values when Gps is disabled
      // And update telemetry and logs
      m_locked = 0; // Reset GPS lock status
      m_fixValid = Fw::On::OFF; // Reset fix validity
      this->log_ACTIVITY_HI_fixValidity(m_fixValid); // Log fix validity
      m_numSentences = 0; // Reset the number of sentences
      this->tlmWrite_numSentences(m_numSentences); // Update telemetry
      this->tlmWrite_latitude(0.0); // Update telemetry
      this->tlmWrite_longitude(0.0); // Update telemetry
      this->tlmWrite_altitude(0.0); // Update telemetry
    }
    // Log the Gps state change
    this->log_ACTIVITY_HI_GpsState(m_GpsEnabled);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Gps
