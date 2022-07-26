function setpaths(add)

%
% USAGE: 
%
%   setpaths(add)
%
% DESCRIPTION:
%
%   Sets all the paths for Matlab_proj. 
%
% INPUTS:
%
%   add: if false or ommitted then setpaths adds all the homer2 paths,
%           specified in getpaths. If true, adds all the homer2 paths 
%           specified in getpaths. 
%
% OUTPUTS:
%
%    


if ~exist('add','var') | isempty(add)
    add = true;
end

paths = getpaths();

rootpath = pwd;
k = find(rootpath=='\');
rootpath(k)='/';

paths_str = '';
err = false;
for ii=1:length(paths)
    paths{ii} = [rootpath, paths{ii}];
    if ~exist(paths{ii}, 'dir')
        err = true;
        continue;
    end
    if isempty(paths_str)
        paths_str = paths{ii};
    else
        paths_str = [paths_str, ';', paths{ii}];
    end
end

if err
    menu('WARNING: The current folder does NOT look like a Matlab_proj root folder. Please change current folder to the root Matlab_proj folder and rerun setpaths.', 'OK');
    paths = {};
    return;
end


if add
    fprintf('ADDED Matlab_proj paths to matlab search paths:\n');
    addpath(paths_str, '-end')    
else
    fprintf('REMOVED Matlab_proj paths from matlab search paths:\n');
    rmpath(paths_str);
end


