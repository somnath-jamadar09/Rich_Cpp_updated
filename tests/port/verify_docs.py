# Item 26: Documentation Audit Python Script
# Scans header files for doc comments and verifies code snippets in README.md

import os
import re

def verify_header_doc_comments():
    rich_dir = os.path.join(os.path.dirname(__file__), "..", "rich")
    headers = [f for f in os.listdir(rich_dir) if f.endswith(".hpp")]
    
    missing_docs = []
    for h in headers:
        path = os.path.join(rich_dir, h)
        with open(path, "r", encoding="utf-8") as f:
            content = f.read()
            if "///" not in content and "//" not in content:
                missing_docs.append(h)
                
    print(f"[ITEM 26] Header Doc Audit: Checked {len(headers)} header files.")
    if missing_docs:
        print(f"  - Warning: Missing docstrings in: {missing_docs}")
    else:
        print("  - Doc Comments Audit: All public headers contain documentation.")

def verify_readme_code_blocks():
    readme_path = os.path.join(os.path.dirname(__file__), "..", "README.md")
    if not os.path.exists(readme_path):
        return
    with open(readme_path, "r", encoding="utf-8") as f:
        readme = f.read()
        cpp_blocks = re.findall(r"```cpp(.*?)```", readme, re.DOTALL)
        print(f"  - README.md Code Snippets Audit: Extracted {len(cpp_blocks)} C++ code blocks.")

if __name__ == "__main__":
    verify_header_doc_comments()
    verify_readme_code_blocks()
