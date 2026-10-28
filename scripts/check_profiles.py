"""Check resolved PlatformIO settings, including inherited driver flags."""
from pathlib import Path
from platformio.project.config import ProjectConfig
config = ProjectConfig.get_instance(str(Path(__file__).resolve().parents[1]/'platformio.ini'))
assert config.get('platformio','default_envs') == ['cyd']
for env in ['cyd','demo','cyd_st7789','demo_st7789']:
    flags = ' '.join(config.get('env:'+env,'build_flags'))
    st = 'st7789' in env
    assert ('-D ST7789_DRIVER=1' in flags) == st, env
    assert ('-D ILI9341_2_DRIVER=1' in flags) != st, env
    assert ('-D DISPLAY_INVERT=0' in flags) == st, env
    assert ('-D DISPLAY_INVERT=1' in flags) != st, env
    assert ('-D THERMOMETER_DEMO=1' in flags) == env.startswith('demo'), env
    for setting in ['TFT_BL=21','ONE_WIRE_PIN=27','SPI_FREQUENCY=40000000']:
        assert '-D '+setting in flags, (env, setting)
    if st: assert '-D TFT_RGB_ORDER=TFT_BGR' in flags, env
print('All four profiles resolve to exactly one driver and the intended inversion, wiring and demo mode.')
