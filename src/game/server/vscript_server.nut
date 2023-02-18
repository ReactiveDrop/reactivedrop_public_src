//========== Copyright © 2008, Valve Corporation, All rights reserved. ========
//
// Purpose:
//
//=============================================================================

function UniqueString( string = "" )
{
	return DoUniqueString( string.tostring() );
}

function EntFire( target, action, value = null, delay = 0.0, activator = null )
{
	if ( !value )
	{
		value = "";
	}
	
	local caller = null;
	if ( "self" in this )
	{
		caller = self;
		if ( !activator )
		{
			activator = self;
		}
	}
	
	DoEntFire( target.tostring(), action.tostring(), value.tostring(), delay, activator, caller ); 
}

function __ReplaceClosures( script, scope )
{
	if ( !scope )
	{
		scope = getroottable();
	}
	
	local tempParent = { getroottable = function() { return null; } };
	local temp = { runscript = script };
	delegate tempParent : temp;
	
	temp.runscript()
	foreach( key,val in temp )
	{
		if ( typeof(val) == "function" && key != "runscript" )
		{
			printl( "   Replacing " + key );
			scope[key] <- val;
		}
	}
}

__OutputsPattern <- regexp("^On.*Output$");

function ConnectOutputs( table )
{
	const nCharsToStrip = 6;
	foreach( key, val in table )
	{
		if ( typeof( val ) == "function" && __OutputsPattern.match( key ) )
		{
			//printl(key.slice( 0, nCharsToStrip ) );
			table.self.ConnectOutput( key.slice( 0, key.len() - nCharsToStrip ), key );
		}
	}
}

function IncludeScript( name, scope = null )
{
	if ( scope == null )
	{
		scope = this;
	}
	return ::DoIncludeScript( name, scope );
}

//---------------------------------------------------------
// Text dump this scope's contents to the console.
//---------------------------------------------------------
function __DumpScope( depth, table )
{
	local indent=function( count )
	{
		local i;
		for( i = 0 ; i < count ; i++ )
		{
			print("   ");
		}
	}

	foreach(key, value in table)
	{
		indent(depth);
		print( key );
		switch (type(value))
		{
			case "table":
				print("(TABLE)\n");
				indent(depth);
				print("{\n");
				__DumpScope( depth + 1, value);
				indent(depth);
				print("}");
				break;
			case "array":
				print("(ARRAY)\n");
				indent(depth);
				print("[\n")
				__DumpScope( depth + 1, value);
				indent(depth);
				print("]");
				break;
			case "string":
				print(" = \"");
				print(value);
				print("\"");
				break;
			default:
				print(" = ");
				print(value);
				break;
		}
		print("\n");
	}
}

function DeepPrintTable( debugTable, prefix = "" )
{
	if (prefix == "")
	{
		printl("{");
		prefix = "   ";
	}
	foreach (idx, val in debugTable)
	{
		if ( typeof(val) == "table" )
		{
			printl( prefix + idx + " = \n" + prefix + "{");
			DeepPrintTable( val, prefix + "   " );
			printl(prefix + "}");
		}
		else if ( typeof(val) == "string" )
			printl(prefix + idx + "\t= \"" + val + "\"");
		else
			printl(prefix + idx + "\t= " + val);
	}
	if (prefix == "   ")
		printl("}");
}

function DuplicateTable( srcTable )
{
	local result = clone srcTable;
	foreach( key, val in srcTable )
	{
		if ( typeof( val ) == "table" )
		{
			result[ key ] = DuplicateTable( val );
		}
	}
	return result;
}

function InjectTable( overrideTable, baseTable )
{
	foreach (idx, val in overrideTable)
	{
		if ( typeof(val) == "table" )
		{
			if (! (idx in baseTable) )
			{
				baseTable[idx] <- {}; // make sure there is a table here to inject into in the base
			}
			InjectTable( val, baseTable[idx] );
		}
		else
		{
			if (val == null)
				baseTable.rawdelete(idx); // specify null to remove a key!
			else
				baseTable[idx] <- overrideTable[idx];
		}
	}
}

function StringToVector( str, delimiter = "," )
{
	local vec = Vector( 0, 0, 0 );

	local result = split( str, delimiter );

	vec.x = result[0].tointeger();
	vec.y = result[1].tointeger();
	vec.z = result[2].tointeger();

	return vec;
}

function _entHelper( ent, funcname )
{
	if (ent == null)
		printl("No entity!");
	else
	{
		if (typeof(funcname) == "function")
		{
			funcname(ent);
		}
		else if (typeof(funcname) == "string")
		{
			if (funcname in ent)
				ent[funcname]();
			else
				printl("No " + funcname + " in " + ent.GetName());
		}
		else
			printl("Need to pass a string of a function name or a lambda function, not a " + typeof(funcname));
	}
}

function EntCall( idxorname, funcname )
{
	local hEnt = null;

	if ( typeof(idxorname) == "string" )
	{
		local foundany = false;
		while ( hEnt = Entities.FindByName( hEnt, idxorname ) )
		{
			foundany = true;
			_entHelper( hEnt, funcname );
		}
		if (!foundany)
		{
			while ( hEnt = Entities.FindByClassname( hEnt, idxorname ) )
			{
				foundany = true;
				_entHelper( hEnt, funcname );
			}
		}
		if (!foundany)
			printl("Never saw anything that matched " + idxorname );
	}
	else if ( typeof(idxorname) == "integer" )
	{
		hEnt = EntIndexToHScript( idxorname );
		_entHelper( hEnt, funcname );
	}
}

function Ent( idxorname )
{
	local hEnt = null;
	if ( typeof(idxorname) == "string" )
		hEnt = Entities.FindByName( null, idxorname );
	else if ( typeof(idxorname) == "integer" )
		hEnt = EntIndexToHScript( idxorname );
	if (hEnt)
		return hEnt;
	printl( "Hey! no entity for " + idxorname );
}

function CASW_Marine_GetInvTableOverride( CASW_Marine )
{
	local table = {};
	CASW_Marine.GetInventoryTable( table );
	return table;
}

function ClientPrint( player, target, message, param1 = "", param2 = "", param3 = "", param4 = "" )
{
	DoClientPrint( player, target, message, param1.tostring(), param2.tostring(), param3.tostring(), param4.tostring() );
}
