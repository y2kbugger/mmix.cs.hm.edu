%	Testing the mmixal extensions
	
	LOC	Data_Segment
	GREG	@

%	Testing negative BYTE, WYDE, or TETRA values

	BYTE	0,254,#FF,255          % OK
	BYTE	256,#100,#1FF,#FFFFFF  % not OK
	BYTE	0,-1,-127,-128	       % now OK
	BYTE	-129,-255,-256,-100000 % not OK


%	Testing the FILE instruction

a	FILE				% filename missing
b	FILE	""			% empty filename
c	FILE	"doesnt.exist"  	% file does not exist  
d	FILE	"testalex.empty"	% file exists but is empty
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
