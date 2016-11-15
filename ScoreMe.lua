--#!/bin/lua
--parse the folder of wav files and score each of them
--then collect the scores to form the final report
--
--1  put wav44to16.exe, lua.exe, lua.dll and ScoreMe.lua to D:\Cortana\Score\
--2  wav tracks extraced will be put to D:\Cortana\Input\
--3  data path is independent of this script


--+++++++++++++
--users params+
--+++++++++++++
DataPath = "Z:\\20160000\\"
CortanaInputPath = "C:\\Cortana\\Input\\"
CortanaScorePath = "C:\\Cortana\\Score\\"
Unscaled = false


--+++++++++++++
--resource    +
--+++++++++++++
local task = {}   --stores absolute paths of all wav files to be scored
local result = {} --result txt files of each task



--+++++++++++++
--work flow   +
--+++++++++++++
os.execute("dir /b/s " .. DataPath .. "*.wav > todo.list") --get the wav files for splitting, resample and levelling

--push the absolute path to table tasks
print("info: files to be scored:")
for line in io.lines("todo.list") do
    table.insert(task, line)
	print(line)
end
print("=========================")
print("= " .. #task .. " files found")
print("=========================")

--iterate over all tasks
for i=1,#task do
	os.execute("del /f /q " .. CortanaScorePath .. "Output\\ScoreResults\\*.txt")
	os.execute("rmdir /s /q " .. CortanaInputPath)
	os.execute("mkdir " .. CortanaInputPath)
	
	if Unscaled then
		os.execute("wav44to16.exe " .. task[i] .. " " .. CortanaInputPath .. " unscaled")
	else
		os.execute("wav44to16.exe " .. task[i] .. " " .. CortanaInputPath)
	end
	os.execute("score " .. CortanaInputPath)

	--how to collect score results?
	os.execute("type " .. CortanaScorePath .. "Output\\ScoreResults\\*.txt > " .. task[i] .. ".report")

	print(i .. "/" .. #task .. " done")
end



--generate the final report
os.execute("dir /b/s " .. DataPath .. "*.report > result.list")

print("info: results to be collected:")
for line in io.lines("result.list") do
    table.insert(result, line)
	print(line)
end

os.execute("echo " .. os.date() .. " > report.txt")
for i=1,#result do
	os.execute("echo " .. "[" .. i .. "] >> report.txt")
	os.execute("echo " .. task[i] .. " >> report.txt")
	os.execute("type " .. result[i] .. " >> report.txt")
end