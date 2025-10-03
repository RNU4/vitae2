import time

start_time = time.time()

total = 0
i = 0
while i < 200000000:
    i+=1
    total += i


end_time = time.time()
execution_time = end_time - start_time
print(f"Execution time: {execution_time:.5f} seconds")
