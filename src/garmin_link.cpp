// $Id$
// garmin_link.cp
// Douglas S. J. De Couto
// May 28, 1998

// Copyright (C) 1998 Douglas S. J. De Couto
// <decouto@lcs.mit.edu>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "garmin_link.h"

#include <cassert>
#ifdef DBUG
#include <iostream>
using namespace std;
#endif

namespace garmin {

  const uint8 dle = 16;
  const uint8 etx = 3;

  const int max_random_bytes = 255; 
  // max number of bytes to scan trying to sync to a packet frame

  /////////////////////

  packet_id
    link_layer::get_packet(uint8 *data, uint8 &sz) throw (not_possible, timeout)
    {
      if (!m_init) {
	throw not_possible("The garmin link layer is not initialized");
      }
  
      try {
    
	int curr_try = 0;
	while (true) { 
	  // loop until we get a good packet...
      
	  curr_try++;
	  if (curr_try > m_retries) {
	    throw not_possible("Too many bad packets encountered  while waiting for a good packet");
	  }
      
	  uint8 id;
	  int bytes_scanned = 0;
	  while (true) {
	    if (bytes_scanned > max_random_bytes) {
	      throw not_possible("Too many random bytes encountered while waiting for a packet");
	    }
			
	    // scan until we get first byte of a packet...
	    id = m_pl->getbyte();
	    bytes_scanned++;
	    if (id == dle) {
	      // could be it, but could be dle in chunk of leftover packet...
	      id = m_pl->getbyte();
	      bytes_scanned++;
	      // valid packet start dle is never followed by dle or etx
	      if (id == dle) {
		// ``dle stuffing'' in packet data, ignore...
		continue;
	      }
	      if (id == etx) {
		// end of old packet frame, ignore...
		continue;
	      } 
	      break;
	    }
	  }
		
	  sz = m_pl->getbyte();
	  if (sz == dle) {
	    sz = m_pl->getbyte();
	    if (sz != dle) {
#ifdef DBUG
	      cerr << "link_layer::get_packet -- missing dle stuffing in packet size" << endl;
#endif
	      goto bad_packet;
	    }
	  }
	  uint8 cksum;
	  cksum = id + sz;
	  for (uint8 i = 0; i < sz; i++) {
	    data[i] = m_pl->getbyte();
	    cksum += data[i];
	    if (data[i] == dle) {
	      // check for and ignore correct dle stuffing byte
	      uint8 b = m_pl->getbyte();
	      if (b != dle) {
#ifdef DBUG
		cerr << "link_layer::get_packet -- missing dle stuffing in packet data" << endl;
#endif	
		goto bad_packet;
	      }
	    }
	  }
		
	  cksum = 256 - cksum;
	  uint8 packet_cksum;
	  packet_cksum = m_pl->getbyte();
	  if (packet_cksum == dle) {
	    packet_cksum = m_pl->getbyte();
	    if (packet_cksum != dle) {
#ifdef DBUG
	      cerr << "link_layer::get_packet -- missing dle stuffing in packet cksum" << endl;
#endif
	      goto bad_packet;
	    }
	  }
	
	  if (cksum != packet_cksum) {
#ifdef DBUG
	    cerr << "link_layer::get_packet -- bad checksum" << endl;
#endif	
	    goto bad_packet;
	  }
		
	  uint8 b1, b2;
	  b1 = m_pl->getbyte();
	  b2 = m_pl->getbyte();
	  if (b1 != dle || b2 != etx) {
#ifdef DBUG
	    cerr << "link_layer::get_packet -- bad packet framing" << endl;
	    cerr << "link_layer::get_packet -- id is " << id << endl;
	    cerr << "link_layer::get_packet -- sz is " << sz << endl;
	    cerr << "link_layer::get_packet -- data is: ";
	    for (int i = 0; i < sz; i++) {
	      cerr << data[i] << " ";
	    }
	    cerr << endl;
	    cerr << "link_layer::get_packet -- cksum is " << packet_cksum << endl;
	    cerr << "link_layer::get_packet -- dle byte is " << b1 << endl;
	    cerr << "link_layer::get_packet -- etx byte is " << b2 << endl;
#endif	
	    goto bad_packet;
	  }
		
	  // if we got this far, we got the packet ok, so send ACK
	  m_pl->putbyte(dle);
	  m_pl->putbyte(pid_ack_byte);
	  m_pl->putbyte(2);
	  m_pl->putbyte(id);
	  m_pl->putbyte(0); // msb of 16 bit id must be 0, since all ids are only one byte... 
	  uint8 ack_cksum;
	  ack_cksum = 256 - (pid_ack_byte + 2 + id);
	  if (ack_cksum == dle) {
	    m_pl->putbyte(dle);
	  }
	  m_pl->putbyte(ack_cksum);
	  m_pl->putbyte(dle);
	  m_pl->putbyte(etx);
	
	  // should check that we recognize the id of the packet we just received....
	  // *** fill in later ***
	  return (packet_id) id; 
		
	bad_packet:
#ifdef DBUG
	  cerr << "link_layer::get_packet -- sending NAK" << endl;
#endif
	  // send NAK and loop again
	  m_pl->putbyte(dle);
	  m_pl->putbyte(pid_nack_byte);
	  m_pl->putbyte(2);
	  m_pl->putbyte(id); // what if we didn't get the id right?
	  m_pl->putbyte(0);
	  uint8 nack_cksum = 256 - (pid_nack_byte + 2 + id);
	  if (nack_cksum == dle) {
	    m_pl->putbyte(dle);
	  }
	  m_pl->putbyte(nack_cksum);
	  m_pl->putbyte(dle);
	  m_pl->putbyte(etx);

	}
      } // try
      catch (not_possible &ex) {
	throw;
      }
      catch (timeout &ex) {
	throw;
      }
      return (packet_id) 0;
    }

  /////////////////////

  void
    link_layer::put_packet(packet_id id, uint8 *data, uint8 sz) throw (not_possible, timeout)
    {
      if (!m_init) {
	throw not_possible("The garmin link layer is not initialized");
      }

      try {
		
	int curr_try = 0;
	while (true) {
	  curr_try++;
	  if (curr_try > m_retries) {
	    throw not_possible("Too many retries while sending packet");
	  }
			
	  // try to send packet...			
	  // put packet header
	  m_pl->putbyte(dle);
	  m_pl->putbyte((uint8) id);
	  if (sz == dle) {
	    m_pl->putbyte(dle);
	  }
	  m_pl->putbyte(sz);
		
	  uint8 cksum = id + sz;
		
	  // put data
	  for (uint8 i = 0; i < sz; i++) {
	    if (data[i] == dle) {
	      m_pl->putbyte(dle);
	    }
	    m_pl->putbyte(data[i]);
	    cksum += data[i];
	  }
		
	  cksum = 256 - cksum;
		
	  // trailer
	  m_pl->putbyte(cksum);
	  if (cksum == dle) {
	    m_pl->putbyte(dle);
	  }
	  m_pl->putbyte(dle);
	  m_pl->putbyte(etx);
		
	  // wait for ack.
	  // assume all packets are delivered in order, and that an ack/nak follows each 
	  // packet received by the gps.  i.e. next incoming packet is our ACK or NACK.
	  uint8 rcv_id;
	  int bytes_scanned = 0;
	  while (true) {
	    if (bytes_scanned > max_random_bytes) {
	      throw not_possible("Too many random bytes encountered while waiting for ack packet");
	    }
	    rcv_id = m_pl->getbyte();
	    bytes_scanned++;
	    if (rcv_id == dle) {
	      rcv_id = m_pl->getbyte();
	      bytes_scanned++;
	      if (rcv_id == dle) {
		continue;
	      }
	      if (rcv_id == etx) {
		continue;
	      } 
	      break;
	    }
				// should check for a timeout here, in case of garbage data stream...
	  }
		
	  uint8 rcv_sz = m_pl->getbyte();
	  if (rcv_sz == dle) {
	    rcv_sz = m_pl->getbyte();
	    if (rcv_sz != dle) {
#ifdef DBUG
	      cerr << "link_layer::put_packet -- missing dle stuffing in reply packet size" << endl;
#endif
	      goto bad_packet;
	    }
	  }
			
		
	  uint8 rcv_cksum;
	  rcv_cksum = rcv_id + rcv_sz;
	
	  for (uint8 i = 0; i < rcv_sz; i++) {
	    uint8 b = m_pl->getbyte();
	    if (b == dle) {
	      b = m_pl->getbyte();
	      if (b != dle) {
#ifdef DBUG
		cerr << "link_layer::put_packet -- missing dle stuffing in reply packet data" << endl;
#endif
		goto bad_packet;			
	      }
	    }
	    rcv_cksum += b;
	    m_buf[i] = b;
	  }
		
	  rcv_cksum = 256 - rcv_cksum;
	  uint8 b;
	  b = m_pl->getbyte();
	  if (b == dle) {
	    b = m_pl->getbyte();
	    if (b != dle) {
#ifdef DBUG
	      cerr << "link_layer::put_packet -- missing dle stuffing in reply packet cksum" << endl;
#endif
	      goto bad_packet;
	    }
	  }
	  if (b != rcv_cksum) {
#ifdef DBUG
	    cerr << "link_layer::put_packet -- bad cksum in reply packet" << endl;
#endif
	    goto bad_packet;
	  }
	  uint8 b1, b2;
	  b1 = m_pl->getbyte();
	  b2 = m_pl->getbyte();			
	  if (b1 != dle || b2 != etx) {
#ifdef DBUG
	    cerr << "link_layer::put_packet -- bad packet framing in reply packet" << endl;
	    cerr << "link_layer::put_packet -- id is " << rcv_id << endl;
	    cerr << "link_layer::put_packet -- sz is " << rcv_sz << endl;
	    cerr << "link_layer::put_packet -- data is: ";
	    for (int i = 0; i < rcv_sz; i++) {
	      cerr << m_buf[i] << " ";
	    }
	    cerr << endl;
	    cerr << "link_layer::put_packet -- cksum is " << rcv_cksum << endl;
	    cerr << "link_layer::put_packet -- dle byte is " << b1 << endl;
	    cerr << "link_layer::put_packet -- etx byte is " << b2 << endl;
#endif	
	    goto bad_packet;
	  }
	  if (rcv_id == pid_nack_byte || rcv_id == pid_ack_byte) {
				// check packet size and data
	    if (rcv_sz != 2) {
#ifdef DBUG
	      cerr << "link_layer::put_packet -- ack/nack packet has bad size" << endl;
#endif
	      goto bad_packet;
	    }
	    if (m_buf[0] != (uint8) id) {
	      // this ack/nack is not for what we just transmitted, resend our packet.
#ifdef DBUG
	      cerr << "link_layer::put_packet -- ack/nack packet is for some other packet id" << endl;
#endif
	      continue;
	    }
	    if (rcv_id == pid_ack_byte) {
	      break;
	    }
	    if (rcv_id == pid_nack_byte) {
#ifdef DBUG
	      cerr << "link_layer::put_packet -- got nack, retransmitting" << endl;
#endif
	      continue; // retransmit...
	    }
	  }
	  else {
				// not an ack or nack reply, send our packet again...
#ifdef DBUG
	    cerr << "link_layer::put_packet -- got some packet besides ack/nack (" 
		 << (unsigned int) rcv_id << "), retransmitting" << endl;
	    /*	if (rcv_id == 10) {
		sint16 *buf16 = (sint16 *) m_buf;
		cout << "cmnd == " << buf16[0] << endl;
		} */
#endif
	    continue;
	  }
			
	bad_packet:
	  // send NACK for bad reply packet; the garmin 12XL seems to implement this, the 
	  // garmin spec is not clear to me.
	  m_pl->putbyte(dle);
	  m_pl->putbyte(pid_nack_byte);
	  m_pl->putbyte(2);
	  m_pl->putbyte(rcv_id);
	  m_pl->putbyte(0); 
	  uint8 nack_cksum = 256 - (pid_nack_byte + 2 + id);
	  if (nack_cksum == dle) {
	    m_pl->putbyte(dle);
	  }
	  m_pl->putbyte(nack_cksum); 
	  m_pl->putbyte(dle);
	  m_pl->putbyte(etx);
			
	  // loop again to retransmit original packet
	} // while
      } // try
      catch (not_possible) {
	throw;
      }
      catch (timeout) {
	throw;
      }
	
    }

  ///////////////////

} // namespace
