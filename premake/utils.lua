function GetOS()
	return package.config:sub(1,1) == "\\" and "win" or "unix"
end