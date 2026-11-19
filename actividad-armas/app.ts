// ===
// Clases modelo (agregación)

class Arma {
  constructor(private nombre: string, private danio: number) {}

  atacar(): void {
    console.log(`⚔️ Ataque con ${this.nombre}: ${this.danio} puntos de daño`);
  }

  getDanio(): number { return this.danio; }
  getNombre(): string { return this.nombre; }
}

class Habilidad {
  constructor(private nombre: string, private poder: number) {}

  usar(): void {
    console.log(`✨ Se utiliza ${this.nombre}: ${this.poder} puntos de poder`);
  }

  getNombre(): string { return this.nombre; }
}

class Personaje {
  private vidaMaxima: number;

  constructor(
    private nombre: string,
    private vida: number,
    private arma: Arma,
    private habilidad: Habilidad
  ) {
    this.vidaMaxima = vida;
  }

  atacar(): void {
    if (this.estaMuerto()) {
      console.log(`💀 ${this.nombre} está muerto y no puede atacar.`);
      return;
    }
    console.log(`${this.nombre} ataca:`);
    this.arma.atacar();
  }

  usarHabilidad(): void {
    if (this.estaMuerto()) {
      console.log(`💀 ${this.nombre} está muerto y no puede usar habilidades.`);
      return;
    }
    console.log(`${this.nombre} utiliza una habilidad:`);
    this.habilidad.usar();
  }

  cambiarArma(nuevaArma: Arma): void {
    this.arma = nuevaArma;
    console.log(`${this.nombre} ahora utiliza ${nuevaArma.getNombre()}`);
  }

  cambiarHabilidad(nuevaHabilidad: Habilidad): void {
    this.habilidad = nuevaHabilidad;
    console.log(`${this.nombre} ahora equipa ${nuevaHabilidad.getNombre()}`);
  }

  recibirDanio(danio: number): void {
    if (this.estaMuerto()) {
      console.log(`💀 ${this.nombre} ya está derrotado.`);
      return;
    }

    this.vida -= danio;

    if (this.vida <= 0) {
      this.vida = 0;
      console.log(`${this.nombre} recibió ${danio} puntos de daño.`);
      console.log(`☠️ ¡El ${this.nombre} ha sido derrotado! (Vida: 0)`);
    } else {
      console.log(`${this.nombre} recibió ${danio} puntos de daño.`);
      console.log(`Vida restante: ${this.vida}`);
    }
  }

  reiniciar(): void {
    this.vida = this.vidaMaxima;
    console.log(`🔄 ¡${this.nombre} ha resucitado con ${this.vida} de vida!`);
  }

  estaMuerto(): boolean {
    return this.vida <= 0;
  }

  getNombre(): string { return this.nombre; }
  getVida(): number { return this.vida; }
}

// ===
// Redireccion de consola al dom

const outputDiv = document.getElementById("console-output")!;

const originalLog = console.log;
console.log = (...args: any[]) => {
  originalLog(...args);
  const p = document.createElement("p");
  p.style.margin = "2px 0";
  p.textContent = args.join(" ");
  outputDiv.appendChild(p);
  outputDiv.scrollTop = outputDiv.scrollHeight;
};

// ===
// Agregación

const armas: Record<string, Arma> = {
  espada: new Arma("Espada de fuego", 30),
  arco: new Arma("Arco mágico", 20)
};

const habilidades: Record<string, Habilidad> = {
  curacion: new Habilidad("Curación", 25),
  escudo: new Habilidad("Escudo mágico", 15)
};

const jugador = new Personaje("Guerrero", 100, armas.espada, habilidades.curacion);

// ===
// ELEMENTOS DEL DOM Y EVENTOS

const btnAtacar = document.getElementById("btn-atacar") as HTMLButtonElement;
const btnHabilidad = document.getElementById("btn-habilidad") as HTMLButtonElement;
const btnDanio = document.getElementById("btn-danio") as HTMLButtonElement;
const btnReiniciar = document.getElementById("btn-reiniciar") as HTMLButtonElement;

function actualizarEstadoBotones(): void {
  const muerto = jugador.estaMuerto();
  btnAtacar.disabled = muerto;
  btnHabilidad.disabled = muerto;
  btnDanio.disabled = muerto;
  btnReiniciar.style.display = muerto ? "block" : "none";
}

document.getElementById("select-arma")!.addEventListener("change", (e) => {
  const target = e.target as HTMLSelectElement;
  jugador.cambiarArma(armas[target.value]);
});

document.getElementById("select-habilidad")!.addEventListener("change", (e) => {
  const target = e.target as HTMLSelectElement;
  jugador.cambiarHabilidad(habilidades[target.value]);
});

btnAtacar.addEventListener("click", () => jugador.atacar());

btnHabilidad.addEventListener("click", () => jugador.usarHabilidad());

btnDanio.addEventListener("click", () => {
  jugador.recibirDanio(20);
  actualizarEstadoBotones();
});

btnReiniciar.addEventListener("click", () => {
  jugador.reiniciar();
  actualizarEstadoBotones();
});

// Estado inicial
actualizarEstadoBotones();