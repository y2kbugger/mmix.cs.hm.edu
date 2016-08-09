%	Testing the mmixal extensions
	
	LOC	Data_Segment
	GREG	@

%	Testing the FILE instruction

a	FILE				% filename missing
	FILE	"testalex.unterminated  % filename unterminated
b	FILE	""			% empty filename
c	FILE	"doesnt.exist"  	% file does not exist  
d	FILE	"testalex.empty"	% file exists but is empty
	FILE	"../alex/testalex.nonempty"	% file exists and is non empty
	FILE	"/home/ruckert/mmixhome/src/mmixware/branches/alex/testalex.nonempty"	% file exists and is non empty
e	FILE	"testalex.nonempty"	% file exists and is non empty
	BYTE	0			% terminate string
f	BYTE	"Hello world",10	% equivalent to previous FILE instruction
	BYTE	0			% terminate string
g	FILE	"testalex.mms"		% this very file


	LOC	#100
Main	IS	@

	LDA	$255,e
	TRAP	0,Fputs,StdOut	% Program outputs file testalex.nonempt

	LDA	$255,f
	TRAP	0,Fputs,StdOut	% Program outputs "Hello world".10

	LDA	$255,g
	TRAP	0,Fputs,StdOut	% Program outputs itself.

	TRAP	0,Halt,0
