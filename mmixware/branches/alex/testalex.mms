%	Testing the mmixal extensions
	
	LOC	Data_Segment
	GREG	@

%	Testing floating point constants
	OCTA	1.0,+1.0,-1.0		% OK
	OCTA	2.0e+2,100.0e-2,5e-1	% OK with exponent
	TETRA	1.0,+1.0,-1.0		% OK short floats
	OCTA	forward			% OK forward reference for OCTA
	TETRA	forward			% not possible with TETRA
	
	% Testing precission
	OCTA	1.00000011920928955078125		% OK 1+2^-23
	TETRA	1.00000011920928955078125		% just OK
	TETRA	1.000000119209289550781249		% just not OK

	OCTA	1.000000059604644775390625		% OK 1+2^-24
	TETRA	1.000000059604644775390625		% TETRA is imprecise

	; 1+2^52
	OCTA	1.0000000000000002220446049250313080847263336181640625	%OK
	OCTA	1.00000000000000022204460492503130808472633361816406249	%not OK
	; 1+2^53
	OCTA	1.00000000000000011102230246251565404236316680908203125	%not OK

	OCTA	1.0e-1,0.1			% always imprecise
	TETRA	1.0e-1,0.1			% truncated twice

	% only unary + and - are allowed
	OCTA	1.0+1
	OCTA	1.0*1
	OCTA	1.0-1
	OCTA	~1.0
	OCTA	$1.0    

forward	IS	1.0				% here forward is defined.



%	Testing negative BYTE, WYDE, or TETRA values

	BYTE	0,254,#FF,255          % OK
	BYTE	256,#100,#1FF,#FFFFFF  % not OK
	BYTE	0,-1,-127,-128	       % now OK
	BYTE	-129,-255,-256,-100000 % not OK


%	Testing the FILE instruction

a	FILE				% filename missing
	FILE	"testalex.unterminated  % filename unterminated
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
