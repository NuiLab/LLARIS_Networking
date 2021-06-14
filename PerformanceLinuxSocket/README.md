Performance TCP Server:
  Run this on the applicable host machine, then run the appropriate client version as well (TCP, or UDP to match TCP, UDP server program, respectively).
  This is not a general purpose server "as-is" and is written to have a strict back and forth between blocking receive calls with the host and client.
      Specifically, client will upload a message to the server and get that message echoed back - recording the delta times in the process
      However, when the client sends a message length of 1, then the server expects to receive message timings from the client to be converted to uint32_t
          and then written to a file before closing the server program.
          
          
          
Original overview (4/21/2021):
  The maximum size (before failure) in bytes to send to a server through TCP was around 218 whereas UDP was about 215. I performed iterative testing from 
      these maximum sizes down to 512 bytes (closed interval) - bisecting the message size after collecting 100 samples of delta times between sending and 
      receiving the same message. Therefore, TCP had 10 cycles of testing and UDP had 7 cycles.


  To avoid complications with reported times between machines, a two-way (round trip) approach was taken so that delta times were consistent within the 
      HoloLens 2. The delta time was calculated locally to the HL2 by (end_time  - start_time) where the start time was stored just before sending the message, 
      and the end time was stored just after verifying a non-zero length message was received.
      [Addendum/Correction to original design: overhead added on server and client side to test for stream size in TCP using first four bytes]
