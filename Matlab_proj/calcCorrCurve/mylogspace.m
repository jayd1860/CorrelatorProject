function y = mylogspace(d1, d2, n)
%LOGSPACE Logarithmically spaced vector.
%   LOGSPACE(X1, X2) generates a row vector of 50 logarithmically
%   equally spaced points between decades 10^X1 and 10^X2.  If X2
%   is pi, then the points are between 10^X1 and pi.
%
%   LOGSPACE(X1, X2, N) generates N points.
%   For N = 1, LOGSPACE returns 10^X2.
%
%   Class support for inputs X1,X2:
%      float: double, single
%
%   See also LINSPACE, COLON.

%   Copyright 1984-2012 The MathWorks, Inc. 
%   $Revision: 5.11.4.6 $  $Date: 2012/02/09 20:58:06 $

% Change by Jay Dubb combines logspace with call to linspace and change
% matlab matrix operations to for loop, for easy portability to C/C++. 

if d2 == pi || d2 == single(pi) 
    d2 = log10(d2);
end

n1 = n-1;

c = (d2 - d1)*(n1-1); % opposite signs may cause overflow
if isinf(c)
    for ii=0:n1
        y(ii+1) = 10^(d1 + (d2/n1)*ii - (d1/n1)*ii);
    end
else
    for ii=0:n1
        y(ii+1) = 10^(d1 + ii*(d2 - d1)/n1);
    end
end

if ~isempty(y)
    y(1) = 10^d1;
    y(n) = 10^d2;
end


