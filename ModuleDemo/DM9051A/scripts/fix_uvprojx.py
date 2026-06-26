from pathlib import Path
import sys

p = Path(sys.argv[1])
text = p.read_text(encoding='utf-8')

replacements = [
    (
        "..\dm9051_driver\examples\lwip_mh2203_demo\main_uip_mh2203_demo.c",
        "..\dm9051_driver\examples\uip_mh2203_demo\main_uip_mh2203_demo.c",
    ),
    (
        "ETH_IF_LWIP_DEMO=mh2030a",
        "ETH_IF_LWIP_DEMO=mh2203",
    ),
]

for old, new in replacements:
    count = text.count(old)
    if count:
        text = text.replace(old, new)
        print(f'Replaced {count}x: {old[:40]}... -> {new[:40]}...')
    else:
        print(f'Not found: {old[:40]}')

p.write_text(text, encoding='utf-8')
print('Done')
