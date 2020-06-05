function notify(message) {
  console.log(JSON.stringify({ type: "message", message }));
}

const squares = document.getElementById("squares");
const count = document.getElementById("count");

function detachAll(children) {
  const offsets = children.map(({ offsetTop: top, offsetLeft: left }) => ({
    top,
    left,
  }));

  children.forEach((child, i) => {
    child.style.position = "absolute";
    child.style.top = offsets[i].top + "px";
    child.style.left = offsets[i].left + "px";
  });
}

function receive(msg) {
  if (msg.type === "reset") {
    // Trucos para animar el reseteo, no importante
    const children = [...squares.children];
    detachAll(children);
    children.forEach((e) => {
      e.classList.remove("in");
      setTimeout(() => (e.style.opacity = 0), 20);
    });

    setTimeout(() => children.forEach((c) => c.remove()), 300);
  } else if (msg.type === "add")
    squares.appendChild(
      Object.assign(document.createElement("div"), {
        className: `square ${msg.kind} in`,
      })
    );
  else if (msg.type === "remove") {
    // Trucos para animar la salida de un elemento
    const child = [...squares.children].reverse().find((e) => !e.willBeRemoved);
    detachAll([child]);
    child.willBeRemoved = true;
    child.classList.remove("in");
    child.classList.add("out");

    // Esto debería ser lo único necesario
    setTimeout(() => squares.removeChild(child), 300);
  }

  count.innerText = msg.count;
}
