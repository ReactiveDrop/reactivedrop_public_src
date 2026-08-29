#pragma once

class bf_read;

// Metadata decoded from one CLC_Move body. The command counts come from the
// encoded header, while the command-number fields come from decoded commands
// in wire order. Range endpoints are meaningful only for valid moves with
// new commands.
struct ASRD_GNS_MoveMetadata
{
	// True only when the complete body and every declared usercmd were decoded,
	// and the declared payload was consumed exactly within the supplied limit.
	bool valid;

	// True when the encoded new-command count is nonzero; a body with no new
	// commands leaves this false.
	bool has_new_commands;

	// Header-declared counts and their sum. The total includes both backup and
	// new commands decoded from the body.
	int new_command_count;
	int backup_command_count;
	int total_command_count;

	// Command number of the first (oldest) new command in wire order.
	int first_new_command_number;
	// Command number of the last new command in wire order, used as the
	// inclusive upper endpoint for range classification.
	int highest_new_command_number;

	// Number of bits consumed after the 6-bit message type, including the
	// CLC_Move header fields and exactly the declared number of payload bits
	// (the payload length is not measured in bytes). The parser probes a copy;
	// the caller owns advancing the original reader by this amount.
	int consumed_bits;
};

enum ASRD_GNS_MoveRangeClass
{
	// Invalid metadata or no new commands; there is no new-command range.
	ASRD_GNS_MOVE_NO_NEW_COMMANDS = 0,
	// A new-command range with no previously accepted command to compare.
	ASRD_GNS_MOVE_FIRST_ACCEPTED,
	// The first new command is exactly one after the last accepted command.
	ASRD_GNS_MOVE_CONTIGUOUS,
	// The first new command is more than one after the last accepted command.
	ASRD_GNS_MOVE_GAP,
	// The highest new command is at or before the last accepted command.
	ASRD_GNS_MOVE_STALE,
	// The range starts at or before the last accepted command and ends after it.
	ASRD_GNS_MOVE_PARTIAL_OVERLAP,
};

// Reads and validates one CLC_Move body beginning immediately after the
// 6-bit clc_Move type. availableBits is the body limit in bits from the
// reader's current position. On success, out receives the header counts,
// decoded new-command endpoints, and exact body bits consumed. The input
// reader is never advanced by this function, and the declared payload length
// bounds every ReadUsercmd call.
bool ASRD_GNS_ParseCLCMoveBody( bf_read *readerAfterType,
	int availableBits, ASRD_GNS_MoveMetadata *out );

// Classifies the inclusive [first_new_command_number,
// highest_new_command_number] range against the last accepted command number.
// Backup commands and moves without new commands do not affect the
// classification.
ASRD_GNS_MoveRangeClass ASRD_GNS_ClassifyMoveRange(
	const ASRD_GNS_MoveMetadata &move, bool hasLastAccepted,
	int lastAcceptedCommandNumber );

// Returns the count of command numbers strictly between the last accepted and
// first new command. Returns zero for invalid/no-new metadata, when no prior
// command exists, and for stale or overlapping ranges.
int ASRD_GNS_ComputeMoveDropNumber(
	const ASRD_GNS_MoveMetadata &move, bool hasLastAccepted,
	int lastAcceptedCommandNumber );
