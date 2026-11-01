import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile

binary = pathlib.Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='rescateusb-cli-') as temp:
    root = pathlib.Path(temp)
    image = bytearray(1024)
    image[446 + 4] = 0x83
    image[446 + 8:446 + 12] = (1).to_bytes(4, 'little')
    image[446 + 12:446 + 16] = (1).to_bytes(4, 'little')
    image[510:512] = b'\x55\xaa'
    image_path = root / 'disk image test.img'
    image_path.write_bytes(image)
    expected = hashlib.sha256(image).hexdigest()
    report_path = root / 'report.json'

    result = subprocess.run([str(binary), str(image_path), '--json', '--expected-sha256', expected], capture_output=True, text=True)
    assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
    report = json.loads(result.stdout)
    assert report['valid'] is True and report['partition_table'] == 'MBR'
    assert report['partitions'] == 1 and report['expected_sha256_match'] is True
    assert report['sha256'] == expected

    result = subprocess.run([str(binary), str(image_path), '--json-out', str(report_path), '--debug'], capture_output=True, text=True)
    assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
    assert 'debug:' in result.stderr.lower()
    file_report = json.loads(report_path.read_text(encoding='utf-8'))
    assert file_report['sha256'] == expected and file_report['valid'] is True
    assert 'diagnóstico' in result.stdout.lower()

    result = subprocess.run([str(binary), str(image_path), '--expected-sha256', '0' * 64], capture_output=True, text=True)
    assert result.returncode == 3, (result.returncode, result.stdout, result.stderr)

    result = subprocess.run([str(binary), str(root / 'missing.img')], capture_output=True, text=True)
    assert result.returncode == 2, (result.returncode, result.stdout, result.stderr)

print('CLI integration tests passed: JSON escaping, report file, debug channel, SHA match/mismatch and missing input')
