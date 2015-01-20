#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Serialization
#include <boost/test/unit_test.hpp>
#include "Utils/src/communication.h"


int compare(unsigned char *c1, unsigned char *c2) {
    for (int i=0; i< INT_LENGTH; ++i) {
        if ( *c1 != *c2 ) { 
	  return 1;
	}
        c1++; 
	c2++;
    }
    return 0;
}

int equals_requests(request_data req1, request_data req2) {
  if ((req1.time != req2.time) || (req1.method != req2.method) || (req1.receiver_ip.s_addr != req2.receiver_ip.s_addr)) {
    return 1;
  }
  return 0;
}

int equals_commands(command comm1, command comm2) {
  if ((comm1.type != comm2.type) || (comm1.time != comm2.time)) {
    return 1;
  }
  return 0;
}


BOOST_AUTO_TEST_SUITE( Serialization )

BOOST_AUTO_TEST_CASE( int_serialization )
{
    unsigned char expected_serialized_int[] = { 0x00, 0x01, 0xe2, 0x40 };
    BOOST_CHECK( compare(serialize_int(123456) , expected_serialized_int) == 0 );
}

BOOST_AUTO_TEST_CASE( int_deserialization )
{
    unsigned char serialized_int[] = { 0x00, 0x01, 0xe2, 0x40 };
    unsigned int expected_int = static_cast<unsigned int>(123456);
    BOOST_CHECK(deserialize_int(serialized_int) == expected_int );
}

BOOST_AUTO_TEST_CASE( int_serialization_deserialization )
{
    unsigned int number = static_cast<unsigned int>(123456);
    unsigned char* serialized_int = serialize_int(number);
    unsigned int deserialized_number = deserialize_int(serialized_int);
    BOOST_CHECK(number == deserialized_number);
}

BOOST_AUTO_TEST_CASE( request_serialization )
{
    request_data req; 
    req.time = 1421667051; 
    req.method = PUT;
    req.receiver_ip.s_addr = inet_addr("151.143.99.201");
    unsigned char expected_serialized_request[] = { 0x54, 0xbc, 0xea, 0xeb, 0x03, 0xc9, 0x63, 0x8f, 0x97 };
    BOOST_CHECK( compare(serialize_request(req), expected_serialized_request) == 0);
}

BOOST_AUTO_TEST_CASE( request_deserialization )
{
    unsigned char serialized_request[] = { 0x54, 0xbc, 0xf2, 0x63, 0x06, 0xcd, 0xab, 0x75, 0x34 };
    request_data expected_req; 
    expected_req.time = 1421668963; 
    expected_req.method = TRACE;
    expected_req.receiver_ip.s_addr = inet_addr("52.117.171.205");
    BOOST_CHECK(equals_requests(deserialize_request(serialized_request), expected_req) == 0);
}

BOOST_AUTO_TEST_CASE( request_serialization_deserialization  )
{
    request_data req; 
    req.time = 1421671173; 
    req.method = POST;
    req.receiver_ip.s_addr = inet_addr("222.161.171.232");
    unsigned char* serialized_req = serialize_request(req);
    request_data deserialized_req = deserialize_request(serialized_req);
    BOOST_CHECK(equals_requests(deserialized_req, req)==0);
}

BOOST_AUTO_TEST_CASE( command_serialization )
{
    command comm;
    comm.type = START;
    comm.time = 1421671645;
    unsigned char expected_comm[] = { 0x01, 0x54, 0xbc, 0xfc, 0xdd };
    BOOST_CHECK(compare(serialize_command(comm), expected_comm)==0);
}

BOOST_AUTO_TEST_CASE( command_deserialization )
{
    unsigned char serialized_comm[] = { 0x02, 0x54, 0xbd, 0x01, 0xea };
    command expected_comm;
    expected_comm.type = STOP;
    expected_comm.time = 1421672938;
    BOOST_CHECK(equals_commands(deserialize_command(serialized_comm), expected_comm)==0);
}

BOOST_AUTO_TEST_CASE( command_serialization_deserialization )
{
    command comm;
    comm.type = GET_DATA;
    comm.time = 1421673405;
    unsigned char* serialized_comm = serialize_command(comm);
    command deserialized_comm = deserialize_command(serialized_comm);
    BOOST_CHECK(equals_commands(deserialized_comm, comm)==0);
}

BOOST_AUTO_TEST_SUITE_END()