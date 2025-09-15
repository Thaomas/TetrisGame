'use strict';

(function () {
	const connectBtn = document.getElementById('connectBtn');
	const statusEl = document.getElementById('status');
	// const rawEl = document.getElementById('raw');
	const baudEl = document.getElementById('baud');
	const scaleEl = document.getElementById('scale');
	const canvas = document.getElementById('canvas');
	const ctx = canvas.getContext('2d');

	const GRID_ROWS = 22;
	const GRID_COLS = 10;

	let port = null;
	let reader = null;
	let reading = false;
	let buffer = '';

	function setStatus(text) {
		statusEl.textContent = text;
	}

	function ensureCanvasSize() {
		const scale = Math.max(10, Math.min(40, parseInt(scaleEl.value || '20', 10)));
		canvas.width = GRID_COLS * scale;
		canvas.height = GRID_ROWS * scale;
		ctx.imageSmoothingEnabled = false;
	}

	function renderGridFromAscii(frame) {
		// frame includes borders '+' '-' '|' and 22 lines of content plus borders
		const lines = frame.split(/\r?\n/).filter(Boolean);
		// Expect at least 24 lines: top border, 22 grid, bottom border
		if (lines.length < 24) return;

		const gridLines = lines.slice(1, 1 + GRID_ROWS); // skip top border
		const scale = canvas.width / GRID_COLS;
		ctx.clearRect(0, 0, canvas.width, canvas.height);

		for (let row = 0; row < GRID_ROWS; row++) {
			const line = gridLines[row];
			if (line.length < GRID_COLS + 2) continue;
			for (let col = 0; col < GRID_COLS; col++) {
				const ch = line[col + 1]; // skip left '|'
				switch (ch) {
					case '1':
						ctx.fillStyle = '#e74c3c'; // red
						break;
					case '2':
						ctx.fillStyle = '#3498db'; // blue
						break;
					case '3':
						ctx.fillStyle = '#f1c40f'; // yellow
						break;
					case '4':
						ctx.fillStyle = '#9b59b6'; // purple
						break;
					case '5':
						ctx.fillStyle = '#e67e22'; // orange
						break;
					case '6':
						ctx.fillStyle = '#1abc9c'; // cyan
						break;
					case '7':
						ctx.fillStyle = '#35c46a'; // white
						break;
					default:
						ctx.fillStyle = '#0e1621'; // background
						break;
				}
				ctx.fillRect(col * scale, row * scale, scale, scale);
				ctx.strokeStyle = '#1e2a3a';
				ctx.strokeRect(col * scale + 0.5, row * scale + 0.5, scale - 1, scale - 1);
			}
		}
	}

	function tryExtractFrame() {
		// A frame starts with "+----------+" and ends with bottom border plus blank line
		console.log(buffer);
		const topIndex = buffer.indexOf('+----------+');
		if (topIndex === -1) return null;
		// Ensure we start from top border
		if (topIndex > 0) buffer = buffer.slice(topIndex);

		// Need 1 top + 22 rows + 1 bottom + trailing newline
		const expectedLines = 24; // not counting the extra blank line printed after
		let pos = 0;
		let lines = 0;
		for (let i = 0; i < buffer.length; i++) {
			if (buffer[i] === '\n') {
				lines++;
				if (lines >= expectedLines) {
					pos = i + 1;
					break;
				}
			}
		}
		if (pos === 0) return null; // incomplete
		const frame = buffer.slice(0, pos);
		buffer = buffer.slice(pos);
		return frame;
	}

	async function readLoop() {
		const textDecoder = new TextDecoderStream();
		const readableStreamClosed = port.readable.pipeTo(textDecoder.writable);
		const stream = textDecoder.readable;
		reader = stream.getReader();
		reading = true;
		setStatus('Reading...');
		try {
			while (reading) {
				const { value, done } = await reader.read();
				if (done) break;
				if (value) {
					buffer += value;
					// rawEl.textContent = buffer.slice(-2000);
					let frame;
					while ((frame = tryExtractFrame())) {
						renderGridFromAscii(frame);
					}
				}
			}
		} catch (err) {
			console.error(err);
			setStatus('Error: ' + err.message);
		} finally {
			try { reader.releaseLock(); } catch { }
			try { await readableStreamClosed; } catch { }
		}
	}

	async function connectSerial() {
		if (!('serial' in navigator)) {
			alert('Web Serial not supported. Use Chrome/Edge on desktop.');
			return;
		}
		try {
			const baudRate = parseInt(baudEl.value || '115200', 10);
			port = await navigator.serial.requestPort();
			await port.open({ baudRate });
			setStatus('Connected @ ' + baudRate + ' baud');
			connectBtn.textContent = 'Disconnect';
			ensureCanvasSize();
			readLoop();
		} catch (err) {
			console.error(err);
			setStatus('Connect failed: ' + err.message);
		}
	}

	async function disconnectSerial() {
		reading = false;
		try { if (reader) await reader.cancel(); } catch { }
		try { if (port) await port.close(); } catch { }
		port = null;
		reader = null;
		setStatus('Disconnected');
		connectBtn.textContent = 'Connect Serial';
	}

	connectBtn.addEventListener('click', async () => {
		if (port) {
			await disconnectSerial();
		} else {
			await connectSerial();
		}
	});

	scaleEl.addEventListener('change', ensureCanvasSize);
	ensureCanvasSize();
})();


