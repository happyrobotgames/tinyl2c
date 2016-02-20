
function exec(line)
	func,err = load(line)
	if func == nil then
		print("Error: " .. err)
		return
	end
	
	status,res = pcall(func)
	if status == false then
		print("Error: " .. res)
		return
	end
	
	if res != nil then
		print(res)
	end
end

while true do

	local line = io.read()
	
	if line == nil then 
		print("Exiting due to nil line")
		break 
	end
	
	if(line == "quit") then
		print("Exiting due to quit")
		break
	end
	
	exec(line)
	 
end