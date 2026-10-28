"""Package previously built PlatformIO binaries. Run from the repository root."""
from pathlib import Path
import hashlib
import re
import shutil
import subprocess
import sys
from platformio.package.manager.tool import ToolPackageManager

root = Path(__file__).resolve().parents[1]
version = re.search(r'THERMOMETER_VERSION "([^"]+)"', (root/'include/FirmwareVersion.h').read_text()).group(1)
dest = root/'dist'
dest.mkdir(exist_ok=True)
for env, suffix in [('cyd', ''), ('demo', '-demo')]:
    build = root/'.pio/build'/env
    framework = Path(ToolPackageManager().get_package('framework-arduinoespressif32').path)
    boot_app = framework/'tools/partitions/boot_app0.bin'
    output = dest/f'CYD-Thermometer-v{version}{suffix}-full.bin'
    subprocess.run([sys.executable, str(Path(ToolPackageManager().get_package('tool-esptoolpy').path)/'esptool.py'), '--chip', 'esp32', 'merge_bin', '-o', str(output), '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', '4MB', '0x1000', str(build/'bootloader.bin'), '0x8000', str(build/'partitions.bin'), '0xe000', str(boot_app), '0x10000', str(build/'firmware.bin')], check=True)
    if env=='cyd': shutil.copyfile(build/'firmware.bin',dest/f'CYD-Thermometer-v{version}-app.bin')
files = sorted(dest.glob('*.bin'))
(dest/'SHA256SUMS.txt').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n' for p in files))
print('Packaged:', ', '.join(p.name for p in files))
