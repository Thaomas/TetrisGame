'use strict';

(function () {
	const connectBtn = document.getElementById('connectBtn');
	const statusEl = document.getElementById('status');
	const roomEl = document.getElementById('room');
	const scaleEl = document.getElementById('scale');
	const canvas = document.getElementById('canvas');
	const scoreEl = document.getElementById('scoreValue');
	const bufferEl = document.getElementById('bufferValue');
	const bufferPreview = document.getElementById('bufferPreview');
	const tetrominoGridEl = null;
	const ctx = canvas.getContext('2d');

	const GRID_ROWS = 22;
	const GRID_COLS = 10;

	let socket = null;
	let asciiBuffer = '';
	// Copy of Arduino/main/shapes.h tetrominoes definitions (7 pieces, 4 rotations, 4x4 each)
	const tetrominoes = [
		// I
		[
			[0,0,1,0],[0,0,1,0],[0,0,1,0],[0,0,1,0]
		],
		// O
		[
			[0,0,0,0],[0,1,1,0],[0,1,1,0],[0,0,0,0]
		],
		// T
		[
			[0,0,0,0],[1,1,1,0],[0,1,0,0],[0,0,0,0]
		],
		// S
		[
			[0,0,0,0],[0,1,1,0],[1,1,0,0],[0,0,0,0]
		],
		// Z
		[
			[0,0,0,0],[1,1,0,0],[0,1,1,0],[0,0,0,0]
		],
		// J
		[
			[0,0,0,0],[1,1,1,0],[0,0,1,0],[0,0,0,0]
		],
		// L
		[
			[0,0,0,0],[1,1,1,0],[1,0,0,0],[0,0,0,0]
		],
	];

	function colorForPiece(index) {
		switch (index) {
			case 0: return '#35c4ff'; // I - cyan
			case 1: return '#f1c40f'; // O - yellow
			case 2: return '#9b59b6'; // T - purple
			case 3: return '#2ecc71'; // S - green
			case 4: return '#e74c3c'; // Z - red
			case 5: return '#3498db'; // J - blue
			case 6: return '#e67e22'; // L - orange
			default: return '#e6eef8';
		}
	}

	function renderTetrominoesGallery() {}

	function renderBufferedPreview(bufferIndex) {
		if (!bufferPreview) return;
		const ctxPrev = bufferPreview.getContext('2d');
		const cellSize = Math.floor(Math.min(bufferPreview.width, bufferPreview.height) / 4);
		ctxPrev.clearRect(0, 0, bufferPreview.width, bufferPreview.height);
		ctxPrev.fillStyle = '#0a0e14';
		ctxPrev.fillRect(0, 0, bufferPreview.width, bufferPreview.height);
		if (bufferIndex < 0 || bufferIndex >= tetrominoes.length) return;
		const rotation0 = tetrominoes[bufferIndex];
		const color = colorForPiece(bufferIndex);
		// Center the 4x4 in the preview canvas
		const offsetX = Math.floor((bufferPreview.width - cellSize * 4) / 2);
		const offsetY = Math.floor((bufferPreview.height - cellSize * 4) / 2);
		for (let r = 0; r < 4; r++) {
			for (let c = 0; c < 4; c++) {
				if (rotation0[r][c]) {
					ctxPrev.fillStyle = color;
					ctxPrev.fillRect(offsetX + c * cellSize, offsetY + r * cellSize, cellSize, cellSize);
					ctxPrev.strokeStyle = '#1e2a3a';
					ctxPrev.strokeRect(offsetX + c * cellSize + 0.5, offsetY + r * cellSize + 0.5, cellSize - 1, cellSize - 1);
				}
			}
		}
	}


	const PacketType = {
		GAME_STATE_UPDATE: 0,
		GAME_SCORE_UPDATE: 1,
		GAME_BUFFER_UPDATE: 2,
		GAME_END: 3,
		GAME_START: 4
	};


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
		if (!(bytes instanceof Uint8Array) || bytes.length !== (GRID_ROWS * GRID_COLS)) return;
		renderGridCells((row, col) => String(bytes[row * GRID_COLS + col]));
	}

	function asciiFrameFromBinary(bytes) {
		if (!(bytes instanceof Uint8Array) || bytes.length !== (GRID_ROWS * GRID_COLS)) return '';
		let out = '+----------+';
		for (let r = 0; r < GRID_ROWS; r++) {
			out += '\n|';
			for (let c = 0; c < GRID_COLS; c++) {
				out += String(bytes[r * GRID_COLS + c]);
			}
			out += '|';
		}
		out += '\n+----------+';
		console.log(out);
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
			renderTetrominoesGallery();
		};

		socket.onmessage = (event) => {
			// Expect binary data: first byte = packet type, rest = payload
			const handleBytes = (bytes) => {
				if (!(bytes instanceof Uint8Array) || bytes.length === 0) {
					console.warn('Unexpected WS payload');
					return;
				}
				const packetType = bytes[0];
				const packetData = bytes.slice(1);
				
				switch (packetType) {
					case PacketType.GAME_STATE_UPDATE:
					try {
						console.log(`Grid update for room ${room}: ${packetData.length} bytes`);
						// asciiFrameFromBinary(packetData);	
						renderGridFromBinary(packetData);
						} catch (e) {
							console.error('Failed to decode grid:', e);
						}
						break;
				case PacketType.GAME_SCORE_UPDATE:
					// Expect single byte score
					if (packetData.length > 0) {
						const score = packetData[0];
						scoreEl.textContent = String(score);
						console.log(scoreEl.textContent);
					}
					break;
					case PacketType.GAME_BUFFER_UPDATE:
						console.log("Buffer update");
					if (packetData.length > 0) {
						const buffer = packetData[0];
						bufferEl.textContent = String(buffer);
						renderBufferedPreview(buffer);
						console.log(bufferEl.textContent);
					}
					break;
					case PacketType.GAME_END:
						break;
					case PacketType.GAME_START:
						// Reset handled server-side; nothing to do here yet
						break;
				}
			};

			if (event.data instanceof ArrayBuffer) {
				handleBytes(new Uint8Array(event.data));
			} else if (event.data instanceof Blob) {
				event.data.arrayBuffer().then(buf => handleBytes(new Uint8Array(buf)));
			} else {
				console.warn('Unsupported WS message type');
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
			try { socket.close(); } catch { }
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


