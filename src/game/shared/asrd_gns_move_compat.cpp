#include "cbase.h"
#include "tier1/bitbuf.h"
#include "protocol.h"
#include "usercmd.h"
#include "asrd_gns_move_compat.h"

#include <string.h>

bool ASRD_GNS_ParseCLCMoveBody( bf_read *readerAfterType,
	int availableBits, ASRD_GNS_MoveMetadata *out )
{
	if ( !readerAfterType || !out || availableBits < 0 )
		return false;

	memset( out, 0, sizeof( *out ) );

	// Probe a copy so the original reader remains at the first body bit after
	// the CLC_Move type for the later CLC_Move::ReadFromBuffer() call.
	bf_read headerReader = *readerAfterType;

	const int bodyStartBit = headerReader.GetNumBitsRead();
	const int newCommandCount =
		(int)headerReader.ReadUBitLong( NUM_NEW_COMMAND_BITS );
	const int backupCommandCount =
		(int)headerReader.ReadUBitLong( NUM_BACKUP_COMMAND_BITS );
	const int payloadBitLength = headerReader.ReadWord();

	if ( headerReader.IsOverflowed() )
		return false;

	if ( newCommandCount < 0 ||
		newCommandCount > MAX_NEW_COMMANDS ||
		backupCommandCount < 0 ||
		backupCommandCount > MAX_BACKUP_COMMANDS )
	{
		return false;
	}

	const int totalCommandCount =
		newCommandCount + backupCommandCount;
	const int payloadStartBit = headerReader.GetNumBitsRead();

	// payloadStartBit is the absolute bit position immediately after the three
	// CLC_Move header fields. payloadBitLength is a bit count, not a byte count.
	// The checks keep the payload end within both this reader and the caller's
	// availableBits limit measured from bodyStartBit.
	if ( payloadBitLength < 0 ||
		payloadBitLength > headerReader.GetNumBitsLeft() ||
		bodyStartBit + ( payloadStartBit - bodyStartBit ) + payloadBitLength >
		bodyStartBit + availableBits )
	{
		return false;
	}

	// StartReading's nBits argument is the absolute maximum bit position from
	// the supplied base pointer, not a length relative to iStartBit. Pass the
	// payload end as that limit so ReadUsercmd cannot consume the next message
	// or byte padding after this CLC_Move.
	bf_read commandReader;
	commandReader.StartReading( headerReader.GetBasePointer(),
		(int)headerReader.TotalBytesAvailable(), payloadStartBit,
		payloadStartBit + payloadBitLength );

	CUserCmd previous;
	previous.Reset();
	CUserCmd decoded[ MAX_NEW_COMMANDS + MAX_BACKUP_COMMANDS ];

	for ( int i = 0; i < totalCommandCount; ++i )
	{
		// Each command is delta-decoded against the preceding command in wire
		// order. ReadUsercmd owns the serialized field grammar.
		ReadUsercmd( &commandReader, &decoded[ i ], &previous );
		if ( commandReader.IsOverflowed() )
			return false;
		previous = decoded[ i ];
	}

	if ( commandReader.IsOverflowed() ||
		commandReader.GetNumBitsRead() !=
			payloadStartBit + payloadBitLength )
	{
		return false;
	}

	out->valid = true;
	out->has_new_commands = newCommandCount > 0;
	out->new_command_count = newCommandCount;
	out->backup_command_count = backupCommandCount;
	out->total_command_count = totalCommandCount;
	out->consumed_bits =
		( payloadStartBit - bodyStartBit ) + payloadBitLength;

	if ( out->has_new_commands )
	{
		// Wire order is oldest backup, newest backup, oldest new, newest new.
		// decoded[ backupCommandCount ] is therefore the
		// first new command, and decoded[ totalCommandCount - 1 ] is the last.
		out->first_new_command_number =
			decoded[ backupCommandCount ].command_number;
		out->highest_new_command_number =
			decoded[ totalCommandCount - 1 ].command_number;
	}

	return true;
}

ASRD_GNS_MoveRangeClass ASRD_GNS_ClassifyMoveRange(
	const ASRD_GNS_MoveMetadata &move, bool hasLastAccepted,
	int lastAcceptedCommandNumber )
{
	// Classification compares the inclusive first/last new-command range with
	// the last accepted command number; backup commands never affect it.
	if ( !move.valid || !move.has_new_commands )
		return ASRD_GNS_MOVE_NO_NEW_COMMANDS;

	if ( !hasLastAccepted )
		return ASRD_GNS_MOVE_FIRST_ACCEPTED;

	if ( move.highest_new_command_number <=
		lastAcceptedCommandNumber )
	{
		return ASRD_GNS_MOVE_STALE;
	}

	if ( move.first_new_command_number <=
		lastAcceptedCommandNumber )
	{
		return ASRD_GNS_MOVE_PARTIAL_OVERLAP;
	}

	if ( move.first_new_command_number ==
		lastAcceptedCommandNumber + 1 )
	{
		return ASRD_GNS_MOVE_CONTIGUOUS;
	}

	return ASRD_GNS_MOVE_GAP;
}

int ASRD_GNS_ComputeMoveDropNumber(
	const ASRD_GNS_MoveMetadata &move, bool hasLastAccepted,
	int lastAcceptedCommandNumber )
{
	// Only command numbers strictly between the last accepted and first new
	// command are drops. Stale and overlapping ranges therefore report zero.
	if ( !move.valid || !move.has_new_commands || !hasLastAccepted )
		return 0;

	const int dropped =
		move.first_new_command_number -
		lastAcceptedCommandNumber - 1;
	return dropped > 0 ? dropped : 0;
}
