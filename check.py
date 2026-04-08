import re

def compare_logs(file1_path, file2_path):
    # This pattern finds "CYC:" followed by any numbers and replaces it with "CYC: 0"
    # This "zeros out" the cycle count for comparison purposes only.
    cyc_reset_pattern = re.compile(r'CYC:\s*\d+')

    # This pattern finds any sequence of tabs/multiple spaces and turns them into one space
    whitespace_pattern = re.compile(r'\s+')

    try:
        with open(file1_path, 'r') as f1, open(file2_path, 'r') as f2:
            for line_num, (line1, line2) in enumerate(zip(f1, f2), 1):
                # 1. Normalize CYC value to "0" in both strings
                proc1 = cyc_reset_pattern.sub('CYC: 0', line1.strip())
                proc2 = cyc_reset_pattern.sub('CYC: 0', line2.strip())

                # 2. Collapse all tabs/extra spaces into a single space
                final1 = whitespace_pattern.sub(' ', proc1)
                final2 = whitespace_pattern.sub(' ', proc2)

                if final1 != final2:
                    print(f"❌ Difference found at line {line_num}:")
                    print(f"File 1: {final1}")
                    print(f"File 2: {final2}")
                    return

        print("✅ Files match (ignoring CYC values and extra whitespace).")

    except FileNotFoundError as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    compare_logs('dump.txt', 'dump2.txt')
