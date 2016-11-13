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
DataPath = "D:\\Temp\\"
CortanaInputPath = "D:\\Cortana\\Input\\"


--+++++++++++++
--resource    +
--+++++++++++++
local task = {}   --stores absolute paths of all wav files to be scored




--+++++++++++++
--work flow   +
--+++++++++++++
os.execute("dir /b/s " .. DataPath .. "*.wav > todolist.txt") --get the wav files for splitting, resample and levelling

--push the absolute path to table tasks
print("info: files to be scored:")
for line in io.lines("todolist.txt") do
    table.insert(task, line)
	print(line)
end
print("=========================")
print("= " .. #task .. " files found")
print("=========================")

--iterate over all tasks
for i=1,#task do
	os.execute("rmdir /s /q " .. CortanaInputPath)
	os.execute("mkdir " .. CortanaInputPath)

	os.execute("wav44to16.exe " .. task[i] .. " " .. CortanaInputPath)
	os.execute("score " .. CortanaInputPath)

	--how to collect score results?


	print(i .. "/" .. #task .. " done")
end
