import subprocess
file_id = 'filename'
local_file_path = '/home/huydang/Downloads/excelsample/6__codestringers.xlsx'
html_file_dir_path = '/home/huydang/Desktop/outputdir'


command = [
    '/media/huydang/HuyDang1/xlsxmagic/result',
    '--origin-file-path', local_file_path,
    '--output-dir', html_file_dir_path,
    '--output-file-name', file_id,
]
try:
    subprocess.check_output(
        command
    )
except subprocess.CalledProcessError as e:
    print(e)
