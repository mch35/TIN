protocol = Proto("httpfreq", "HTTPFreq")

--command fields
local f_type = ProtoField.uint8("comm.type", "Type")
local f_command_time = ProtoField.absolute_time("comm.time", "Time")
--requests number message
local f_length = ProtoField.uint32("req.length", "Requests number")
--request fields
local f_request_time = ProtoField.absolute_time("req.time", "Time")
local f_method = ProtoField.string("req.method", "Method")
local f_ip = ProtoField.ipv4("req.ip", "Receiver IP")
local f_code = ProtoField.string("resp.code", "HTTP code")
--response field
local f_response = ProtoField.uint8("response", "Response")

protocol.fields = { f_type, f_command_time, f_length, f_request_time, f_method, f_ip, f_code, f_response }

local COMMAND_LENGTH = 5
local RESPONSE_LENGTH = 1
local REQUEST_LENGTH = 13
local REQUEST_NUMBER_MESSAGE_LENGTH = 4

local INT_SIZE = 4
local CHAR_SIZE = 1
local CODE_SIZE = 3

local command_types = {
    [1]  = {"START"},
    [2]  = {"STOP"},
    [3]  = {"GET_DATA"},
};
	
local http_methods = {
	[1] = {"GET"},
	[2] = {"POST"},
	[3] = {"PUT"},
	[4] = {"HEAD"},
	[5] = {"DELETE"},
	[6] = {"TRACE"},
	[7] = {"OPTIONS"},
	[8] = {"CONNECT"},
	[9] = {"PATCH"},
	[10] = {"RESPONSE"},
};
	
local client_responses = {
	[1] = {"OK"},
	[2] = {"ERROR"},
};
 
--protocol main dissector function	
function protocol.dissector (buff, packet_info, root)	
	if (buff:len() == COMMAND_LENGTH) then
		dissect_command(buff, packet_info, root)
	elseif (buff:len() == RESPONSE_LENGTH) then
		dissect_response(buff, packet_info, root)
	elseif (buff:len() == REQUEST_NUMBER_MESSAGE_LENGTH) then
		dissect_request_number_message(buff, packet_info, root)
	elseif (buff:len() >= REQUEST_LENGTH) then
		dissect_requests(buff, packet_info, root)
	end
end
	
function dissect_command (buff, packet_info, root)
	packet_info.cols.protocol = protocol.name
	packet_info.cols.info:set("Command")
	
	subtree = root:add(protocol, buff(0))
	local offset = 0
	local type = subtree:add(f_type, buff(offset, CHAR_SIZE))
	if (command_types[buff(0, CHAR_SIZE):uint()] ~= nil) then
	      type:append_text(" (" .. command_types[buff(0, CHAR_SIZE):uint()][1] .. ")")
	else
	      type:append_text("Unknown command")
	end
	offset = offset + CHAR_SIZE
	subtree:add(f_command_time, buff(offset, INT_SIZE)) 
end

function dissect_requests (buff, packet_info, root)
	local offset = buff:len() % REQUEST_LENGTH
	if (offset ~= 0 and offset ~= INT_SIZE) then return end
  
	packet_info.cols.protocol = protocol.name
	packet_info.cols.info:set("Data")

	subtree = root:add(protocol, buff(0))
	if (offset == INT_SIZE) then
		subtree:add(f_length, buff(0, offset))
	end
	while (offset < buff:len()) do
		subtree:add(f_request_time, buff(offset, INT_SIZE))
		offset = offset + INT_SIZE
		local method = subtree:add(f_method, buff(offset, CHAR_SIZE):uint())
		if (http_methods[buff(offset, CHAR_SIZE):uint()] ~= nil) then
		      method:append_text(" (" .. http_methods[buff(offset, CHAR_SIZE):uint()][1] .. ")")
		else 
		      method:append_text("Unknown method")
		end
		offset = offset + CHAR_SIZE
		subtree:add_le(f_ip, buff(offset, INT_SIZE))
		offset = offset + INT_SIZE
		subtree:add(f_code, buff(offset+1, CODE_SIZE))
		offset = offset + INT_SIZE
	end
end

function dissect_response (buff, packet_info, root)
	packet_info.cols.protocol = protocol.name
	subtree = root:add(protocol, buff(0))

	local response = subtree:add(f_response, buff(0, CHAR_SIZE))
	if (client_responses[buff(0, CHAR_SIZE):uint()] ~= nil) then
		response:append_text(" (" .. client_responses[buff(0, CHAR_SIZE):uint()][1] .. ")")
		packet_info.cols.info:set(client_responses[buff(0, CHAR_SIZE):uint()][1])
	else
		response:append_text("Unknown response")
		packet_info.cols.info:set("Unknown response")
	end
 
end

function dissect_request_number_message (buff, packet_info, root)
	packet_info.cols.protocol = protocol.name
	packet_info.cols.info:set("Requests number")
	
	subtree = root:add(protocol, buff(0))
	subtree:add(f_length, buff(0, INT_SIZE))
  
end

function protocol.init()
end

local tcp_dissector_table = DissectorTable.get("tcp.port")
tcp_dissector_table:add(5000, protocol)