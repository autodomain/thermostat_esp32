# -*- coding: utf-8 -*-
"""
Strumento di Aggiornamento Firmware Termostato Smart ESP32
Supporta ESP32-2432S028R (CYD) e AZ-Touch MOD 2.8"
Include Controllo di Versione Automatico (Device + GitHub)
"""

import os
import sys
import re
import json
import queue
import threading
import urllib.request
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import serial
import serial.tools.list_ports
import esptool

APP_TITLE = "Termostato Smart ESP32 - Aggiornamento Firmware v2.5"
APP_VERSION = "2.5.0"
GITHUB_VERSION_URL = "https://raw.githubusercontent.com/autodomain/thermostat_esp32/main/version.json"

def get_resource_path(relative_path):
    """Ottiene il percorso assoluto della risorsa, compatibile con PyInstaller e runtime locale."""
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.dirname(os.path.abspath(__file__)), relative_path)

class FlasherApp:
    def __init__(self, root):
        self.root = root
        self.root.title(APP_TITLE)
        self.root.geometry("660x780")
        self.root.minsize(620, 720)

        self.style = ttk.Style()
        try:
            self.style.theme_use('clam')
        except Exception:
            pass

        self.style.configure(".", font=('Segoe UI', 9))
        self.style.configure("Header.TLabel", font=('Segoe UI', 13, 'bold'), foreground="#0284c7")
        self.style.configure("SubHeader.TLabel", font=('Segoe UI', 9), foreground="#64748b")
        self.style.configure("Card.TLabelframe", background="#ffffff", relief="groove")
        self.style.configure("Card.TLabelframe.Label", font=('Segoe UI', 10, 'bold'), foreground="#1e293b")

        self.log_queue = queue.Queue()
        self.flashing_thread = None
        self.is_flashing = False
        self.is_checking_version = False

        self.detected_device = None
        self.github_version_data = None

        self._build_ui()
        self._refresh_ports()
        self._poll_log_queue()
        self._check_github_version_async()

    def _build_ui(self):
        main_frame = ttk.Frame(self.root, padding=12)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # 1. Header
        hdr_frame = ttk.Frame(main_frame)
        hdr_frame.pack(fill=tk.X, pady=(0, 8))

        title_lbl = ttk.Label(hdr_frame, text="TERMOSTATO SMART ESP32", style="Header.TLabel")
        title_lbl.pack(anchor="w")

        sub_lbl = ttk.Label(
            hdr_frame,
            text=f"Strumento Ufficiale di Aggiornamento &bull; Firmware v{APP_VERSION} &bull; Controllo Versione Integrato",
            style="SubHeader.TLabel"
        )
        sub_lbl.pack(anchor="w", pady=(2, 0))

        # 2. Selezione Porta COM
        port_group = ttk.LabelFrame(main_frame, text=" 1. Porta USB / Seriale ", padding=8)
        port_group.pack(fill=tk.X, pady=(0, 6))

        p_inner = ttk.Frame(port_group)
        p_inner.pack(fill=tk.X)

        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(p_inner, textvariable=self.port_var, state="readonly", font=('Segoe UI', 9))
        self.port_combo.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 6))
        self.port_combo.bind("<<ComboboxSelected>>", lambda e: self._on_port_changed())

        self.btn_refresh = ttk.Button(p_inner, text="🔄 Rileva Porte", command=self._refresh_ports, width=13)
        self.btn_refresh.pack(side=tk.RIGHT)

        self.port_hint_lbl = ttk.Label(
            port_group,
            text="Collega il termostato al PC via USB. La porta COM verra' auto-rilevata.",
            foreground="#64748b", font=('Segoe UI', 8)
        )
        self.port_hint_lbl.pack(anchor="w", pady=(3, 0))

        # 3. Riquadro Controllo Versione
        ver_group = ttk.LabelFrame(main_frame, text=" 2. Controllo Versione & Diagnostica ", padding=8)
        ver_group.pack(fill=tk.X, pady=(0, 6))

        v_grid = ttk.Frame(ver_group)
        v_grid.pack(fill=tk.X, pady=(0, 4))

        # Colonna 1: Info dispositivo
        ttk.Label(v_grid, text="Versione a bordo:", font=('Segoe UI', 9, 'bold')).grid(row=0, column=0, sticky="w", padx=(0, 6))
        self.lbl_dev_ver = ttk.Label(v_grid, text="In attesa di verifica...", foreground="#0284c7")
        self.lbl_dev_ver.grid(row=0, column=1, sticky="w")

        ttk.Label(v_grid, text="Hardware rilevato:", font=('Segoe UI', 9, 'bold')).grid(row=1, column=0, sticky="w", padx=(0, 6))
        self.lbl_dev_hw = ttk.Label(v_grid, text="--", foreground="#64748b")
        self.lbl_dev_hw.grid(row=1, column=1, sticky="w")

        # Colonna 2: Info pacchetto e GitHub
        ttk.Label(v_grid, text="Versione pacchetto:", font=('Segoe UI', 9, 'bold')).grid(row=0, column=2, sticky="w", padx=(16, 6))
        ttk.Label(v_grid, text=f"v{APP_VERSION} (Inclusa)", foreground="#16a34a").grid(row=0, column=3, sticky="w")

        ttk.Label(v_grid, text="Versione GitHub:", font=('Segoe UI', 9, 'bold')).grid(row=1, column=2, sticky="w", padx=(16, 6))
        self.lbl_github_ver = ttk.Label(v_grid, text="Controllo online...", foreground="#64748b")
        self.lbl_github_ver.grid(row=1, column=3, sticky="w")

        # Banner stato / verdetto
        self.ver_banner_frame = tk.Frame(ver_group, bg="#f1f5f9", padx=8, pady=6, relief="ridge", bd=1)
        self.ver_banner_frame.pack(fill=tk.X, pady=(4, 4))

        self.ver_banner_lbl = tk.Label(
            self.ver_banner_frame,
            text="Premi 'Verifica Versione' o seleziona la porta COM per interrogare il termostato.",
            bg="#f1f5f9",
            fg="#475569",
            font=('Segoe UI', 9),
            anchor="w",
            justify=tk.LEFT
        )
        self.ver_banner_lbl.pack(side=tk.LEFT, fill=tk.X, expand=True)

        self.btn_check_ver = ttk.Button(ver_group, text="🔍 Verifica Versione Ora", command=self._check_version_manual)
        self.btn_check_ver.pack(anchor="e", pady=(2, 0))

        # 4. Selezione Modello Hardware
        hw_group = ttk.LabelFrame(main_frame, text=" 3. Modello Hardware ", padding=8)
        hw_group.pack(fill=tk.X, pady=(0, 6))

        self.hw_var = tk.StringVar(value="cyd")
        self.rb_cyd = ttk.Radiobutton(
            hw_group,
            text="ESP32-2432S028R (CYD - Cheap Yellow Display 2.8\") [Standard consigliato]",
            variable=self.hw_var,
            value="cyd"
        )
        self.rb_cyd.pack(anchor="w", pady=1)

        self.rb_az = ttk.Radiobutton(
            hw_group,
            text="AZ-Touch MOD (Display 2.8\" Shield da parete)",
            variable=self.hw_var,
            value="az_touch"
        )
        self.rb_az.pack(anchor="w", pady=1)

        # 5. Opzioni Aggiornamento
        opt_group = ttk.LabelFrame(main_frame, text=" 4. Opzioni di Aggiornamento ", padding=8)
        opt_group.pack(fill=tk.X, pady=(0, 6))

        self.keep_settings_var = tk.BooleanVar(value=True)
        self.chk_keep = ttk.Checkbutton(
            opt_group,
            text="Conserva impostazioni (WiFi, MQTT, Crono, Calibrazione e Storico)",
            variable=self.keep_settings_var,
            command=self._on_keep_settings_toggle
        )
        self.chk_keep.pack(anchor="w")

        self.opt_desc_lbl = ttk.Label(
            opt_group,
            text="Consigliato: aggiorna solo il software senza perdere la password del WiFi o gli orari.",
            foreground="#0284c7", font=('Segoe UI', 8)
        )
        self.opt_desc_lbl.pack(anchor="w", pady=(2, 0))

        # 6. Pulsante Azione
        self.btn_flash = tk.Button(
            main_frame,
            text=f"🚀  AVVIA AGGIORNAMENTO FIRMWARE (v{APP_VERSION})",
            font=('Segoe UI', 11, 'bold'),
            bg="#10b981",
            fg="#ffffff",
            activebackground="#059669",
            activeforeground="#ffffff",
            relief="raised",
            bd=2,
            pady=8,
            cursor="hand2",
            command=self._start_flashing
        )
        self.btn_flash.pack(fill=tk.X, pady=(4, 6))

        # 7. Progresso e Log
        prog_group = ttk.LabelFrame(main_frame, text=" 5. Avanzamento Operazione ", padding=8)
        prog_group.pack(fill=tk.BOTH, expand=True)

        status_row = ttk.Frame(prog_group)
        status_row.pack(fill=tk.X, pady=(0, 3))

        self.status_lbl = ttk.Label(status_row, text="Stato: Pronto", font=('Segoe UI', 9, 'bold'), foreground="#334155")
        self.status_lbl.pack(side=tk.LEFT)

        self.pct_lbl = ttk.Label(status_row, text="0%", font=('Segoe UI', 9, 'bold'), foreground="#0284c7")
        self.pct_lbl.pack(side=tk.RIGHT)

        self.prog_bar = ttk.Progressbar(prog_group, mode="determinate", maximum=100)
        self.prog_bar.pack(fill=tk.X, pady=(0, 6))

        self.log_text = scrolledtext.ScrolledText(
            prog_group,
            height=8,
            font=('Consolas', 8),
            bg="#0f172a",
            fg="#cbd5e1",
            insertbackground="#ffffff",
            relief="flat",
            wrap=tk.WORD
        )
        self.log_text.pack(fill=tk.BOTH, expand=True)

    def _on_keep_settings_toggle(self):
        if self.keep_settings_var.get():
            self.opt_desc_lbl.config(
                text="Consigliato: aggiorna solo il software senza perdere la password del WiFi o gli orari.",
                foreground="#0284c7"
            )
        else:
            self.opt_desc_lbl.config(
                text="ATTENZIONE: Verrà eseguita la cancellazione totale (Ripristino di Fabbrica).",
                foreground="#ef4444"
            )

    def _refresh_ports(self):
        ports = list(serial.tools.list_ports.comports())
        port_list = []
        preferred_index = 0

        for i, p in enumerate(ports):
            desc = p.description or ""
            port_str = f"{p.device} - {desc}"
            port_list.append(port_str)
            d_upper = desc.upper()
            if any(k in d_upper for k in ["CH340", "CP210", "USB-SERIAL", "UART", "ESP32", "SILICON LABS"]):
                preferred_index = i

        self.port_combo['values'] = port_list
        if port_list:
            self.port_combo.current(preferred_index)
            port_name = port_list[preferred_index].split(' - ')[0]
            self.port_hint_lbl.config(
                text=f"Rilevate {len(port_list)} porte. Porta selezionata: {port_name}",
                foreground="#16a34a"
            )
            self._query_device_version_async(port_name)
        else:
            self.port_var.set("")
            self.port_hint_lbl.config(
                text="Nessuna porta seriale trovata! Controlla il cavo USB e installa i driver CH340 / CP2102 se necessario.",
                foreground="#dc2626"
            )
            self._reset_version_ui()

    def _on_port_changed(self):
        port_sel = self.port_var.get()
        if port_sel:
            port = port_sel.split(' - ')[0].strip()
            self._query_device_version_async(port)

    def _reset_version_ui(self):
        self.lbl_dev_ver.config(text="Non connesso", foreground="#64748b")
        self.lbl_dev_hw.config(text="--", foreground="#64748b")
        self.ver_banner_frame.config(bg="#f1f5f9")
        self.ver_banner_lbl.config(
            text="Collega il termostato per avviare il controllo versione.",
            bg="#f1f5f9", fg="#475569"
        )
        self.btn_flash.config(text=f"🚀  AVVIA AGGIORNAMENTO FIRMWARE (v{APP_VERSION})")

    def _check_version_manual(self):
        port_sel = self.port_var.get()
        if not port_sel:
            messagebox.showwarning("Nessuna Porta", "Seleziona prima una porta seriale COM.")
            return
        port = port_sel.split(' - ')[0].strip()
        self._query_device_version_async(port)

    def _query_device_version_async(self, port):
        if self.is_checking_version or self.is_flashing:
            return
        self.is_checking_version = True
        self.lbl_dev_ver.config(text="Interrogazione in corso...", foreground="#0284c7")
        self.ver_banner_lbl.config(text="Lettura versione dal termostato via USB...", bg="#f1f5f9", fg="#0284c7")

        threading.Thread(target=self._worker_query_version, args=(port,), daemon=True).start()

    def _worker_query_version(self, port):
        dev_info = None

        # 1. Tentativo tramite comando Seriale VER (dispositivo acceso ed operativo)
        try:
            s = serial.Serial()
            s.port = port
            s.baudrate = 115200
            s.timeout = 0.8
            s.dtr = False
            s.rts = False
            s.open()
            s.reset_input_buffer()
            s.write(b"VER\n")
            import time
            time.sleep(0.3)
            resp = s.read_all().decode('ascii', errors='ignore')
            s.close()

            for line in resp.splitlines():
                line = line.strip()
                if line.startswith("{") and "termostato_smart" in line and "version" in line:
                    dev_info = json.loads(line)
                    break
        except Exception:
            pass

        self.root.after(0, self._apply_device_version_result, dev_info)

    def _apply_device_version_result(self, dev_info):
        self.is_checking_version = False
        self.detected_device = dev_info

        if dev_info and "version" in dev_info:
            ver = dev_info.get("version", "Sconosciuta")
            hw = dev_info.get("hw", "cyd")
            mac = dev_info.get("mac", "")

            hw_name = "CYD 2.8\" (ESP32-2432S028R)" if hw == "cyd" else "AZ-Touch MOD 2.8\""
            self.lbl_dev_ver.config(text=f"v{ver}", foreground="#0f172a", font=('Segoe UI', 9, 'bold'))
            self.lbl_dev_hw.config(text=f"{hw_name} (MAC: {mac})", foreground="#0284c7")

            # Auto-seleziona il modello hardware corretto
            if hw in ["cyd", "az_touch"]:
                self.hw_var.set(hw)

            # Confronto versione
            if ver == APP_VERSION:
                self.ver_banner_frame.config(bg="#f0fdf4")
                self.ver_banner_lbl.config(
                    text=f"✅ Il termostato è già aggiornato all'ultima versione disponibile (v{APP_VERSION})!",
                    bg="#f0fdf4", fg="#166534", font=('Segoe UI', 9, 'bold')
                )
                self.btn_flash.config(text=f"🔄  REINSTALLA FIRMWARE (v{APP_VERSION})", bg="#0284c7")
            else:
                self.ver_banner_frame.config(bg="#fffbeb")
                self.ver_banner_lbl.config(
                    text=f"⚠️ Rilevata versione precedente v{ver}. Aggiornamento consigliato a v{APP_VERSION}!",
                    bg="#fffbeb", fg="#b45309", font=('Segoe UI', 9, 'bold')
                )
                self.btn_flash.config(text=f"🚀  AGGIORNA DA v{ver} A v{APP_VERSION}", bg="#10b981")
        else:
            self.lbl_dev_ver.config(text="Non identificata (Bootloader o vergine)", foreground="#64748b")
            self.lbl_dev_hw.config(text="ESP32 Standard", foreground="#64748b")
            self.ver_banner_frame.config(bg="#eff6ff")
            self.ver_banner_lbl.config(
                text=f"ℹ️ Termostato pronto per l'installazione del firmware v{APP_VERSION}.",
                bg="#eff6ff", fg="#1e40af"
            )
            self.btn_flash.config(text=f"🚀  INSTALLA FIRMWARE (v{APP_VERSION})", bg="#10b981")

    def _check_github_version_async(self):
        threading.Thread(target=self._worker_check_github, daemon=True).start()

    def _worker_check_github(self):
        gh_data = None
        try:
            req = urllib.request.Request(
                GITHUB_VERSION_URL,
                headers={'User-Agent': 'ESP32-Thermostat-Flasher'}
            )
            with urllib.request.urlopen(req, timeout=3.0) as response:
                if response.status == 200:
                    raw = response.read().decode('utf-8')
                    gh_data = json.loads(raw)
        except Exception:
            pass

        self.root.after(0, self._apply_github_version_result, gh_data)

    def _apply_github_version_result(self, gh_data):
        self.github_version_data = gh_data
        if gh_data and "version" in gh_data:
            gh_ver = gh_data.get("version", "")
            if gh_ver == APP_VERSION:
                self.lbl_github_ver.config(text=f"v{gh_ver} (Allineata)", foreground="#16a34a")
            else:
                self.lbl_github_ver.config(text=f"v{gh_ver} (Disponibile)", foreground="#0284c7")
        else:
            self.lbl_github_ver.config(text="Offline o Non Raggiungibile", foreground="#94a3b8")

    def _append_log(self, text):
        self.log_text.insert(tk.END, text + "\n")
        self.log_text.see(tk.END)

    def _poll_log_queue(self):
        try:
            while True:
                msg_type, data = self.log_queue.get_nowait()
                if msg_type == "LOG":
                    self._append_log(data)
                    match = re.search(r'(\d+(?:\.\d+)?)\s*%', data)
                    if match:
                        try:
                            pct = float(match.group(1))
                            self.prog_bar['value'] = pct
                            self.pct_lbl.config(text=f"{int(pct)}%")
                            if pct < 100:
                                self.status_lbl.config(text=f"Stato: Scrittura flash in corso ({int(pct)}%)...")
                        except ValueError:
                            pass
                elif msg_type == "STATUS":
                    self.status_lbl.config(text=f"Stato: {data}")
                elif msg_type == "DONE":
                    self._on_flashing_done(data)
                self.log_queue.task_done()
        except queue.Empty:
            pass
        self.root.after(100, self._poll_log_queue)

    def _set_ui_state(self, enabled):
        state = tk.NORMAL if enabled else tk.DISABLED
        c_state = "readonly" if enabled else tk.DISABLED
        self.port_combo.config(state=c_state)
        self.btn_refresh.config(state=state)
        self.btn_check_ver.config(state=state)
        self.chk_keep.config(state=state)
        self.rb_cyd.config(state=state)
        self.rb_az.config(state=state)
        if enabled:
            btn_txt = "🚀  AVVIA AGGIORNAMENTO FIRMWARE"
            if self.detected_device and self.detected_device.get("version") == APP_VERSION:
                btn_txt = f"🔄  REINSTALLA FIRMWARE (v{APP_VERSION})"
            self.btn_flash.config(text=btn_txt, state=tk.NORMAL, cursor="hand2")
        else:
            self.btn_flash.config(text="⏳  AGGIORNAMENTO IN CORSO...", bg="#64748b", cursor="wait", state=tk.DISABLED)

    def _start_flashing(self):
        port_sel = self.port_var.get()
        if not port_sel:
            messagebox.showwarning("Nessuna Porta Selezionata", "Collega il termostato e seleziona la porta COM corrispondente.")
            return

        port = port_sel.split(" - ")[0].strip()
        hw_model = self.hw_var.get()
        keep_settings = self.keep_settings_var.get()

        bin_dir = get_resource_path(os.path.join("bin", hw_model))
        bootloader = os.path.join(bin_dir, "bootloader.bin")
        partitions = os.path.join(bin_dir, "partitions.bin")
        boot_app0 = os.path.join(bin_dir, "boot_app0.bin")
        firmware = os.path.join(bin_dir, "firmware.bin")

        for f_path in [bootloader, partitions, boot_app0, firmware]:
            if not os.path.exists(f_path):
                messagebox.showerror(
                    "File Mancante",
                    f"Impossibile trovare il file firmware necessario:\n{f_path}\nVerifica l'installazione."
                )
                return

        if not keep_settings:
            conf = messagebox.askyesno(
                "Conferma Ripristino Totale",
                "Hai scelto di NON conservare le impostazioni.\n\n"
                "Tutti i dati (rete Wi-Fi, credenziali MQTT, orari programmati e calibrazione) verranno CANCELLATI.\n\n"
                "Vuoi procedere ugualmente?"
            )
            if not conf:
                return

        self.is_flashing = True
        self._set_ui_state(False)
        self.log_text.delete('1.0', tk.END)
        self.prog_bar['value'] = 0
        self.pct_lbl.config(text="0%")
        self.status_lbl.config(text="Stato: Connessione all'ESP32...")

        self._append_log("=" * 60)
        self._append_log(f"AVVIO AGGIORNAMENTO - Termostato Smart ESP32 (v{APP_VERSION})")
        self._append_log(f"Modello Hardware: {hw_model.upper()}")
        self._append_log(f"Porta COM: {port}")
        self._append_log(f"Conserva Impostazioni: {'SI' if keep_settings else 'NO (Full Erase)'}")
        if self.detected_device:
            self._append_log(f"Versione Rilevata a Bordo: v{self.detected_device.get('version', 'N/D')}")
        self._append_log("=" * 60)

        self.flashing_thread = threading.Thread(
            target=self._flashing_worker,
            args=(port, hw_model, keep_settings, bootloader, partitions, boot_app0, firmware),
            daemon=True
        )
        self.flashing_thread.start()

    def _flashing_worker(self, port, hw_model, keep_settings, bootloader, partitions, boot_app0, firmware):
        class RedirectWriter:
            def __init__(self, q):
                self.q = q
                self.buf = ""
            def write(self, s):
                if not s:
                    return
                self.buf += s
                while "\n" in self.buf or "\r" in self.buf:
                    nl_pos = self.buf.find("\n")
                    cr_pos = self.buf.find("\r")
                    if nl_pos != -1 and (cr_pos == -1 or nl_pos < cr_pos):
                        line = self.buf[:nl_pos].strip()
                        self.buf = self.buf[nl_pos+1:]
                    else:
                        line = self.buf[:cr_pos].strip()
                        self.buf = self.buf[cr_pos+1:]
                    if line:
                        self.q.put(("LOG", line))
            def flush(self):
                if self.buf.strip():
                    self.q.put(("LOG", self.buf.strip()))
                    self.buf = ""

        redirect = RedirectWriter(self.log_queue)
        old_stdout = sys.stdout
        old_stderr = sys.stderr

        success = False
        err_msg = ""

        try:
            sys.stdout = redirect
            sys.stderr = redirect

            if not keep_settings:
                self.log_queue.put(("STATUS", "Cancellazione memoria flash in corso..."))
                erase_args = [
                    '--chip', 'esp32',
                    '--port', port,
                    '--baud', '460800',
                    '--before', 'default-reset',
                    '--after', 'no-reset',
                    'erase-flash'
                ]
                try:
                    esptool.main(erase_args)
                except SystemExit as se:
                    if se.code != 0 and se.code is not None:
                        raise RuntimeError(f"Errore durante la cancellazione della memoria flash (Exit code {se.code})")

            self.log_queue.put(("STATUS", "Scrittura bootloader, partizioni e firmware..."))
            flash_args = [
                '--chip', 'esp32',
                '--port', port,
                '--baud', '460800',
                '--before', 'default-reset',
                '--after', 'hard-reset',
                'write-flash',
                '-z',
                '--flash-mode', 'dio',
                '--flash-freq', '40m',
                '--flash-size', 'detect',
                '0x1000', bootloader,
                '0x8000', partitions,
                '0xe000', boot_app0,
                '0x10000', firmware
            ]
            try:
                esptool.main(flash_args)
                success = True
            except SystemExit as se:
                if se.code == 0 or se.code is None:
                    success = True
                else:
                    err_msg = f"Processo terminato con codice di errore {se.code}"
        except Exception as ex:
            err_msg = str(ex)
        finally:
            redirect.flush()
            sys.stdout = old_stdout
            sys.stderr = old_stderr

        self.log_queue.put(("DONE", (success, err_msg, port)))

    def _on_flashing_done(self, result):
        success, err_msg, port = result
        self.is_flashing = False
        self._set_ui_state(True)

        if success:
            self.prog_bar['value'] = 100
            self.pct_lbl.config(text="100%")
            self.status_lbl.config(text="Stato: ✅ Aggiornamento Completato con Successo!", foreground="#16a34a")
            self._append_log("\n" + "=" * 60)
            self._append_log(f"✅ OPERAZIONE COMPLETATA! Firmware aggiornato a v{APP_VERSION}.")
            self._append_log("Il termostato si e' riavviato automaticamente con la nuova versione.")
            self._append_log("=" * 60)

            # Ri-interroga la versione per aggiornare il banner
            self.root.after(2000, lambda: self._query_device_version_async(port))

            messagebox.showinfo(
                "Aggiornamento Completato",
                f"Il firmware e' stato aggiornato con successo alla versione v{APP_VERSION}!\n\n"
                "Il termostato si e' riavviato ed e' subito pronto all'uso.\n"
                "Tutte le impostazioni precedenti sono state conservate."
            )
        else:
            self.status_lbl.config(text="Stato: ❌ Errore durante l'aggiornamento", foreground="#dc2626")
            self._append_log("\n" + "!" * 60)
            self._append_log(f"❌ ERRORE: {err_msg}")
            self._append_log("Suggerimenti per risolvere:")
            self._append_log("1. Chiudi eventuali programmi che usano la porta seriale (es. Arduino, Cura).")
            self._append_log("2. Tieni premuto il pulsante BOOT sulla scheda mentre riavvii l'aggiornamento.")
            self._append_log("3. Prova a scollegare e ricollegare il cavo USB.")
            self._append_log("!" * 60)
            messagebox.showerror(
                "Errore Aggiornamento",
                f"Si e' verificato un errore durante la scrittura:\n\n{err_msg}\n\n"
                "Verifica che nessun altro programma stia usando la porta COM, "
                "scollega e ricollega il cavo USB e riprova."
            )

def main():
    root = tk.Tk()
    app = FlasherApp(root)
    root.mainloop()

if __name__ == '__main__':
    main()
