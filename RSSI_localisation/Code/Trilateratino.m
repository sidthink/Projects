clear all; close all;
% INPUTS NEEDED: 
% - Constant values of x1,x2,x3 y1, y2,y3
% - Synched arrays for r1, r2, r3

x1 = 0 ; %constant
y1 = 0; %constant

x2 = 5 ; %constant
y2 = 10; %constant

x3 = 10; %constant
y3 = 0; %constant

r1 = [5 6 7 8] ; %constant
r2 = [12 11 10 9]; %constant
r3 = [1 5 10 15 ]; %constant

%% Equation set of form:
%                        Ax + By = C
%                        Cx + Dy = E
% r1, r2
 
for i = 1:length(r1)
  A = 2*x2 - 2*x1;
  B = 2*y2 - 2*y1;
  C(i) = r1(i)*2 - r2(i)*2 - x1*2 + x2*2 - y1*2 + y2*2;
  D = 2*x3 - 2*x2;
  E(i) = 2*y3 - 2*y2; 
  F(i) = r2(i)*2 - r3(i)*2 - x2*2 + x3*2 - y2*2 + y3*2;
 
 %% solve for x,y (coordinates of source)
  x(i) = (C.*E - F*B) / (E.*A - B*D);
  y(i) = (C*D - A*F) / (B*D - A*E);
end