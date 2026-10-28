"""Package previously built PlatformIO binaries. Run from the repository root."""
from pathlib import Path
import hashlib
import re
import shutil
import subprocess
import sys
from platformio.package.manager.tool import ToolPackageManager

root = Path(__file__).resolve().parents[1]
version = re.search(r'THERMOMETER_VERSION "([^"]+)"', (root/'include/FirmwareVersion.h').read_text(encoding='utf-8')).group(1)
dest = root/'dist'
dest.mkdir(exist_ok=True)
files = []
for env, suffix in [('cyd', '-ili9341'), ('demo', '-ili9341-demo'), ('cyd_st7789', '-st7789'), ('demo_st7789', '-st7789-demo')]:
    build = root/'.pio/build'/env
    framework = Path(ToolPackageManager().get_package('framework-arduinoespressif32').path)
    boot_app = framework/'tools/partitions/boot_app0.bin'
    output = dest/f'CYD-Thermometer-v{version}{suffix}-full.bin'
    subprocess.run([sys.executable, str(Path(ToolPackageManager().get_package('tool-esptoolpy').path)/'esptool.py'), '--chip', 'esp32', 'merge_bin', '-o', str(output), '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', '4MB', '0x1000', str(build/'bootloader.bin'), '0x8000', str(build/'partitions.bin'), '0xe000', str(boot_app), '0x10000', str(build/'firmware.bin')], check=True)
    files.append(output)
    if env in ('cyd', 'cyd_st7789'):
        app = dest/f'CYD-Thermometer-v{version}{suffix}-app.bin'
        shutil.copyfile(build/'firmware.bin',app)
        files.append(app)
files = sorted(files)
(dest/'SHA256SUMS.txt').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n' for p in files))
print('Packaged:', ', '.join(p.name for p in files))
