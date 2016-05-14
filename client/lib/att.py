
# Len of signature in write signed packet
SIGNATURE_LEN = 12

# Attribute Protocol Opcodes
OP_ERROR = 0x01
OP_MTU_REQ = 0x02
OP_MTU_RESP = 0x03
OP_FIND_INFO_REQ = 0x04
OP_FIND_INFO_RESP = 0x05
OP_FIND_BY_TYPE_REQ = 0x06
OP_FIND_BY_TYPE_RESP= 0x07
OP_READ_BY_TYPE_REQ = 0x08
OP_READ_BY_TYPE_RESP = 0x09
OP_READ_REQ = 0x0A
OP_READ_RESP = 0x0B
OP_READ_BLOB_REQ = 0x0C
OP_READ_BLOB_RESP = 0x0D
OP_READ_MULTI_REQ = 0x0E
OP_READ_MULTI_RESP = 0x0F
OP_READ_BY_GROUP_REQ = 0x10
OP_READ_BY_GROUP_RESP = 0x11
OP_WRITE_REQ = 0x12
OP_WRITE_RESP = 0x13
OP_WRITE_CMD = 0x52
OP_PREP_WRITE_REQ = 0x16
OP_PREP_WRITE_RESP = 0x17
OP_EXEC_WRITE_REQ = 0x18
OP_EXEC_WRITE_RESP = 0x19
OP_HANDLE_NOTIFY = 0x1B
OP_HANDLE_IND = 0x1D
OP_HANDLE_CNF = 0x1E
OP_SIGNED_WRITE_CMD = 0xD2


OP_REQUEST = set([
	OP_MTU_REQ, OP_FIND_INFO_REQ, OP_FIND_BY_TYPE_REQ, OP_READ_BY_TYPE_REQ, 
	OP_READ_REQ, OP_READ_BLOB_REQ, OP_READ_MULTI_REQ, OP_READ_BY_GROUP_REQ,  
	OP_WRITE_REQ, OP_PREP_WRITE_REQ, OP_EXEC_WRITE_REQ,
])
def isRequest(opcode):
	return opcode in OP_REQUEST 

OP_RESPONSE = set([
	OP_MTU_RESP, OP_FIND_INFO_RESP, OP_FIND_BY_TYPE_RESP, OP_READ_BY_TYPE_RESP, 
	OP_READ_RESP, OP_READ_BLOB_RESP, OP_READ_MULTI_RESP, OP_READ_BY_GROUP_RESP,  
	OP_WRITE_RESP, OP_PREP_WRITE_RESP, OP_EXEC_WRITE_RESP,
])
def isResponse(opcode):
	return opcode in OP_RESPONSE

# Error codes for Error response PDU
ECODE_INVALID_HANDLE = 0x01
ECODE_READ_NOT_PERM = 0x02
ECODE_WRITE_NOT_PERM = 0x03
ECODE_INVALID_PDU = 0x04
ECODE_AUTHENTICATION = 0x05
ECODE_REQ_NOT_SUPP = 0x06
ECODE_INVALID_OFFSET = 0x07
ECODE_AUTHORIZATION = 0x08
ECODE_PREP_QUEUE_FULL = 0x09
ECODE_ATTR_NOT_FOUND = 0x0A
ECODE_ATTR_NOT_LONG = 0x0B
ECODE_INSUFF_ENCR_KEY_SIZE = 0x0C
ECODE_INVAL_ATTR_VALUE_LEN = 0x0D
ECODE_UNLIKELY = 0x0E
ECODE_INSUFF_ENC = 0x0F
ECODE_UNSUPP_GRP_TYPE = 0x10
ECODE_INSUFF_RESOURCES = 0x11
# Application error
ECODE_IO = 0x80
ECODE_TIMEOUT = 0x81
ECODE_ABORTED = 0x82

MAX_VALUE_LEN = 512
DEFAULT_L2CAP_MTU = 48
DEFAULT_LE_MTU = 23

CID = 4
PSM = 31

# Flags for Execute Write Request Operation
CANCEL_ALL_PREP_WRITES = 0x00
WRITE_ALL_PREP_WRITES = 0x01

# Find Information Response Formats
FIND_INFO_RESP_FMT_16BIT = 0x01
FIND_INFO_RESP_FMT_128BIT = 0x02