'use strict';

(function () {
	const connectBtn = document.getElementById('connectBtn');
	const statusEl = document.getElementById('status');
	const roomEl = document.getElementById('room');
	const scaleEl = document.getElementById('scale');
	const canvas = document.getElementById('canvas');
	const ctx = canvas.getContext('2d');

	const GRID_ROWS = 22;
	const GRID_COLS = 10;

	let socket = null;
	let asciiBuffer = '';

	function setStatus(text) {
		statusEl.textContent = text;
	}

	function ensureCanvasSize() {
		const scale = Math.max(10, Math.min(40, parseInt(scaleEl.value || '20', 10)));
		canvas.width = GRID_COLS * scale;
		canvas.height = GRID_ROWS * scale;
		ctx.imageSmoothingEnabled = false;
	}

	function colorForCell(ch) {
		switch (ch) {
			case '1': return '#e74c3c';
			case '2': return '#3498db';
			case '3': return '#f1c40f';
			case '4': return '#9b59b6';
			case '5': return '#e67e22';
			case '6': return '#1abc9c';
			case '7': return '#35c46a';
			default: return '#0e1621';
		}
	}

	function renderGridCells(getCell) {
		const scale = canvas.width / GRID_COLS;
		ctx.clearRect(0, 0, canvas.width, canvas.height);
		for (let row = 0; row < GRID_ROWS; row++) {
			for (let col = 0; col < GRID_COLS; col++) {
				const ch = getCell(row, col);
				ctx.fillStyle = colorForCell(ch);
				ctx.fillRect(col * scale, row * scale, scale, scale);
				ctx.strokeStyle = '#1e2a3a';
				ctx.strokeRect(col * scale + 0.5, row * scale + 0.5, scale - 1, scale - 1);
			}
		}
	}

	function renderGridFromAscii(frame) {
		const lines = frame.split(/\r?\n/).filter(Boolean);
		if (lines.length < 24) return;
		const gridLines = lines.slice(1, 1 + GRID_ROWS);
		renderGridCells((row, col) => gridLines[row][col + 1] || '0');
	}

	function tryExtractAsciiFrame() {
		const topIndex = asciiBuffer.indexOf('+----------+');
		if (topIndex === -1) return null;
		if (topIndex > 0) asciiBuffer = asciiBuffer.slice(topIndex);
		const expectedLines = 24;
		let pos = 0, lines = 0;
		for (let i = 0; i < asciiBuffer.length; i++) {
			if (asciiBuffer[i] === '\n') {
				lines++;
				if (lines >= expectedLines) { pos = i + 1; break; }
			}
		}
		if (pos === 0) return null;
		const frame = asciiBuffer.slice(0, pos);
		asciiBuffer = asciiBuffer.slice(pos);
		return frame;
	}

	function renderGridFromBinary(bytes) {
		if (!(bytes instanceof Uint8Array) || bytes.length !== GRID_ROWS * GRID_COLS) return;
		renderGridCells((row, col) => String(bytes[row * GRID_COLS + col]));
	}

	function connectWebSocket() {
		const room = (roomEl.value || '').trim();
		if (!room) { alert('Enter a room code to join.'); return; }
		// Deabstracted URL: always connect to ws://localhost:8080/?code=ROOM
		const url = 'ws://localhost:8080/?code=' + encodeURIComponent(room);
		socket = new WebSocket(url);
		socket.binaryType = 'arraybuffer';
		setStatus('Connecting to room ' + room + '...');
		connectBtn.textContent = 'Leave Room';

		socket.onopen = () => {
			setStatus('Connected to room ' + room);
			ensureCanvasSize();
		};

		socket.onmessage = (event) => {
			if (typeof event.data === 'string') {
				asciiBuffer += event.data;
				let frame;
				while ((frame = tryExtractAsciiFrame())) {
					renderGridFromAscii(frame);
				}
			} else if (event.data instanceof ArrayBuffer) {
				renderGridFromBinary(new Uint8Array(event.data));
			}
		};

		socket.onerror = (e) => {
			console.error('WebSocket error', e);
			setStatus('Error');
		};

		socket.onclose = () => {
			setStatus('Disconnected');
			connectBtn.textContent = 'Join Room';
			socket = null;
		};
	}

	function disconnectWebSocket() {
		if (socket) {
			try { socket.close(); } catch {}
			socket = null;
		}
	}

	connectBtn.addEventListener('click', () => {
		if (socket) {
			disconnectWebSocket();
		} else {
			connectWebSocket();
		}
	});

	scaleEl.addEventListener('change', ensureCanvasSize);
	ensureCanvasSize();
})();


