import sys
import glob
import serial
import pyautogui
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
from time import sleep
def controla_jogo(tipo, value):
    """Move o mouse de acordo com o eixo e valor recebidos."""
    if tipo == 1:
        """Roda """
        if value == 1:
            # 
            pyautogui.press('u')
        if value == 2:
            pyautogui.press('i')
    elif tipo == 2:
        pyautogui.press(['k','h'])
    elif tipo == 3:
        if value == 1:
            pyautogui.press('o')
        if value == 2:
            pyautogui.press('f1')
        if value == 3:
            pyautogui.press('f3')
        if value == 4:
            pyautogui.press('p')
    

def controle(ser):
    """
    Loop principal que lê bytes da porta serial em loop infinito.
    Aguarda o byte 0xFF e então lê 2 bytes: tipo de ação (1 byte) + valor (1 bytes).
    """
    while True:
        # Aguardar byte de sincronização
        sync_byte = ser.read(size=1)
        if not sync_byte:
            continue
        if sync_byte[0] == 0xFF:
            # Ler 2 bytes (tipo de ação + valor)
            data = ser.read(size=2)
            if len(data) < 2:
                continue
            print(data)
            tipo, value = parse_data(data)
            controla_jogo(tipo, value)

def serial_ports():
    """Retorna uma lista das portas seriais disponíveis na máquina."""
    ports = []
    if sys.platform.startswith('win'):
        # Windows
        for i in range(1, 256):
            port = f'COM{i}'
            try:
                s = serial.Serial(port)
                s.close()
                ports.append(port)
            except (OSError, serial.SerialException):
                pass
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # Linux/Cygwin
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        # macOS
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Plataforma não suportada para detecção de portas seriais.')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def parse_data(data):
    """Interpreta os dados recebidos do buffer (axis + valor)."""
    tipo = data[0]
    value = int.from_bytes(data[1], byteorder='little', signed=True)
    return tipo, value

def conectar_porta(port_name):
    """Abre a conexão com a porta selecionada e inicia o loop de leitura."""
    if not port_name:
        messagebox.showwarning("Aviso", "Selecione uma porta serial antes de conectar.")
        return

    try:
        ser = serial.Serial(port_name, 115200, timeout=1)

        # Inicia o loop de leitura (bloqueante).
        controle(ser)

    except KeyboardInterrupt:
        print("Encerrando via KeyboardInterrupt.")
    except Exception as e:
        messagebox.showerror("Erro de Conexão", f"Não foi possível conectar em {port_name}.\nErro: {e}")
    finally:
        ser.close()





if __name__ == "__main__":     
    portas_disponiveis = serial_ports()
    if portas_disponiveis:
        conectar_porta(portas_disponiveis[0])

