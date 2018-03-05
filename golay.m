;gen = [ 1 0 0 0 0 0 0 0 0 0 0 0 1 1 0 1 1 1 0 0 0 1 0 1; ...
;        0 1 0 0 0 0 0 0 0 0 0 0 1 0 1 1 1 0 0 0 1 0 1 1; ...
;        0 0 1 0 0 0 0 0 0 0 0 0 0 1 1 1 0 0 0 1 0 1 1 1; ...
;        0 0 0 1 0 0 0 0 0 0 0 0 1 1 1 0 0 0 1 0 1 1 0 1; ...
;        0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 0 1 0 1 1 0 1 1; ...
;        0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 1 0 1 1 0 1 1 1; ...
;        0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 1 0 1 1 0 1 1 1 1; ...
;        0 0 0 0 0 0 0 1 0 0 0 0 0 0 1 0 1 1 0 1 1 1 0 1; ...
;        0 0 0 0 0 0 0 0 1 0 0 0 0 1 0 1 1 0 1 1 1 0 0 1; ...
;        0 0 0 0 0 0 0 0 0 1 0 0 1 0 1 1 0 1 1 1 0 0 0 1; ...
;        0 0 0 0 0 0 0 0 0 0 1 0 0 1 1 0 1 1 1 0 0 0 1 1; ...
;        0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 ];

readlib(GF):
with(Gauss);
G2 := GF(2,1):
M := SquareMatrix(12,G2);
P := M[Input]([[1,1,0,1,1,1,0,0,0,1,0,1],
	[1,0,1,1,1,0,0,0,1,0,1,1],
	[0,1,1,1,0,0,0,1,0,1,1,1],
	[1,1,1,0,0,0,1,0,1,1,0,1],
	[1,1,0,0,0,1,0,1,1,0,1,1],
	[1,0,0,0,1,0,1,1,0,1,1,1],
	[0,0,0,1,0,1,1,0,1,1,1,1],
	[0,0,1,0,1,1,0,1,1,1,0,1],
	[0,1,0,1,1,0,1,1,1,0,0,1],
	[1,0,1,1,0,1,1,1,0,0,0,1],
	[0,1,1,0,1,1,1,0,0,0,1,1],
	[1,1,1,1,1,1,1,1,1,1,1,0]]);
	