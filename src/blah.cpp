def print_table(table):
    print(chr(27) + "[2J")  # clear screen
    for i, row in enumerate(table[1:]):
        for j, val in enumerate(row[::-1]):
            print(f"{'*' if val else '.'}", end="")
        print()


def process_row(table, i):
    for j in range(len(table[0]) - 1, -1, -1):
        present = table[i][j]
        if present and (i != j + 1):
            below = table[i + 1][j] if (i + 1) < len(table) else 0
            if not below:
                if i + 1 < len(table):
                    table[i + 1][j] = table[i][j]
                table[i][j] = 0
            else:
                if j + 1 < len(table[0]):
                    table[i][j + 1] = table[i][j]
                table[i][j] = 0
                table[i + 1][j] = 0
                if i + 2 < len(table):
                    table[i + 2][j] = 1


def main():
    table = [
        [0, 0, 0, 0, 0, 0, 0, 0],  # 0
        [0, 0, 0, 0, 0, 0, 0, 0],  # 0
        [0, 0, 0, 0, 0, 0, 0, 0],  # 1
        [0, 0, 0, 0, 0, 0, 0, 0],  # 2
        [0, 0, 0, 0, 0, 0, 0, 0],  # 3
        [0, 0, 0, 0, 0, 0, 0, 0],  # 4
        [0, 0, 0, 0, 0, 0, 0, 0],  # 5
        [0, 0, 0, 0, 0, 0, 0, 0],  # 6
        [0, 0, 0, 0, 0, 0, 0, 0],  # 7
    ]  # c 0  1  2  3  4  5  6  7

    count = 0
    while True:
        table[0][0] = int(count & 7 == 0)
        count += 1
        print_table(table)

        for i in range(len(table) - 1, -1, -1):
            sleep(0.01)
            process_row(table, i)
