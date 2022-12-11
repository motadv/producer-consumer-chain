import random

# Generate 50 files
for i in range(1, 51):
    # Open the file in write mode
    with open("matrix{}.in".format(i), "w") as f:
        # Write 20 lines with 10 random double values each
        for j in range(20):
            # Generate the random values with 3 decimal places
            values = [round(random.uniform(0, 10), 3) for k in range(10)]
            # Join the values into a single string with spaces
            line = " ".join([str(value) for value in values])
            f.write(line + "\n")
