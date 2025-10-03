local start_time = os.clock()

local total = 0
local i = 0
while i < 200000000 do
    i = i + 1
    total = total + i
end

local end_time = os.clock()
local execution_time = end_time - start_time
print(string.format("Execution time: %.5f seconds", execution_time))
print("Sum :"..total)