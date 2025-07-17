module Gps {
    @ Component to read and process NMEA sentences from a UART-attached GPS receiver
    active component Gps {

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port
        @ TODO
        @ GPS enable command
        async command GpsEnable(newStatus: Fw.On) opcode 0

        @ Count the number of NMEA sentences received
        telemetry numSentences: U32
        @ GGA latitude
        telemetry latitude: F64
        @ GGA longitude
        telemetry longitude: F64
        @ GGA altitude
        telemetry altitude: F64

        @ GPS receiver state: enabled or disabled
        event GpsState(enabled: Fw.On) severity activity high id 1 format "GPS state changed to: {}"
        @ GPS fix validity: valid or invalid
        event fixValidity(valid: Fw.On) severity activity high id 2 format "GPS fix validity changed to: {}"

        @ Port to receive GPS data
        async input port GpsRecv: Drv.ByteStreamRecv
        @ Port to send GPS commands
        output port GpsSend: Drv.ByteStreamSend
        @ Port to return buffers for deallocation
        output port deallocate: Fw.BufferSend

        ##############################################################################
        #### Uncomment the following examples to start customizing your component ####
        ##############################################################################

        # @ Example async command
        # async command COMMAND_NAME(param_name: U32)

        # @ Example telemetry counter
        # telemetry ExampleCounter: U64

        # @ Example event
        # event ExampleStateEvent(example_state: Fw.On) severity activity high id 0 format "State set to {}"

        # @ Example port: receiving calls from the rate group
        # sync input port run: Svc.Sched

        # @ Example parameter
        # param PARAMETER_NAME: U32

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}