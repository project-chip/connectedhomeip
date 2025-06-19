import os
import sys

from flask import Flask, jsonify, request, send_from_directory
from werkzeug.utils import secure_filename

CHIP_HOME = os.getenv("CHIP_HOME")

if not CHIP_HOME:
    print("Error: Please set the CHIP_HOME environment variable.")
    sys.exit(1)

sys.path.insert(0, os.path.join(CHIP_HOME, 'src/controller/python'))
try:
    from chip.tlv import TLVReader
except ImportError:
    print("chip.tlv module not found. Ensure CHIP_HOME is set correctly.")
    sys.exit(1)

app = Flask(__name__)

UPLOAD_FOLDER = '/tmp'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER


@app.route('/')
def index():
    """Serve the HTML page"""
    return send_from_directory('.', 'tlv_viewer.html')


@app.route('/parse_tlv', methods=['POST'])
def parse_tlv():
    """Handle TLV file uploads and return parsed data"""
    if 'tlv_file' not in request.files:
        return jsonify({'error': 'No file part'}), 400

    file = request.files['tlv_file']
    if file.filename == '':
        return jsonify({'error': 'No selected file'}), 400

    filename = secure_filename(file.filename)
    file_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
    file.save(file_path)

    try:
        with open(file_path, 'rb') as f:
            binary_data = f.read()

        binary_data = bytes([0x17]) + binary_data + bytes([0x18])

        t = TLVReader(binary_data)
        data = t.get()

        entries = []
        for tag, entry in data['Any']:
            entries.append(entry)

        formatted_entries = []
        for entry in entries:
            formatted_entries.append({
                'timestamp': entry.get(0, 0),
                'label': entry.get(1, ''),
                'value': entry.get(2, '')
            })

        return jsonify(formatted_entries)
    except Exception as e:
        return jsonify({'error': str(e)}), 500


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000, debug=True)
