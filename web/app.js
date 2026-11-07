const DEFAULT_CODE = `int threshold = 10;\nint val = 5 + 2 * 2;\nif (val < threshold) {\n    val = 99;\n}`;

function nodeColor(name) {
  if (name === "Block") return "#6366f1";
  if (
    name === "if" ||
    name === "else" ||
    name === "while" ||
    name === "for" ||
    name === "cond"
  )
    return "#f59e0b";
  if (
    name === "=" ||
    name === "+" ||
    name === "-" ||
    name === "*" ||
    name === "/" ||
    name === "<" ||
    name === ">" ||
    name === "<=" ||
    name === ">=" ||
    name === "==" ||
    name === "!=" ||
    name === "&&" ||
    name === "||" ||
    name === "!"
  )
    return "#0ea5e9";
  return "#10b981";
}

function renderAST(data) {
  const container = document.getElementById("ast-container");
  const width = container.clientWidth || 700;

  const svg = d3.select("#ast-svg");
  svg.selectAll("*").remove();

  const root = d3.hierarchy(data);
  const treeLayout = d3.tree().nodeSize([95, 85]);
  treeLayout(root);

  let minX = Infinity,
    maxX = -Infinity;
  root.each((d) => {
    if (d.x < minX) minX = d.x;
    if (d.x > maxX) maxX = d.x;
  });

  const g = svg.append("g");

  const zoom = d3
    .zoom()
    .scaleExtent([0.3, 2.5])
    .on("zoom", (event) => g.attr("transform", event.transform));

  svg.call(zoom);

  const treeWidth = maxX - minX;
  const initialX = width / 2 - (minX + treeWidth / 2);
  svg.call(zoom.transform, d3.zoomIdentity.translate(initialX, 50));

  g.selectAll(".link")
    .data(root.links())
    .enter()
    .append("path")
    .attr("class", "link")
    .attr(
      "d",
      d3
        .linkVertical()
        .x((d) => d.x)
        .y((d) => d.y),
    );

  const node = g
    .selectAll(".node")
    .data(root.descendants())
    .enter()
    .append("g")
    .attr("class", "node")
    .attr("transform", (d) => `translate(${d.x},${d.y})`);

  node
    .append("circle")
    .attr("r", (d) =>
      Math.max(20, Math.min(38, d.data.name.length * 4.2 + 10)),
    )
    .style("fill", (d) => nodeColor(d.data.name));

  node
    .append("text")
    .attr("dy", 4)
    .attr("text-anchor", "middle")
    .text((d) => d.data.name);
}

const codeInput = document.getElementById("code-input");
const runBtn = document.getElementById("run-btn");
const errorBox = document.getElementById("error-box");
const evalOutput = document.getElementById("eval-output");
const codegenOutput = document.getElementById("codegen-output");

function showError(msg) {
  errorBox.textContent = msg;
  errorBox.classList.remove("hidden");
}

function clearError() {
  errorBox.textContent = "";
  errorBox.classList.add("hidden");
}

async function runPipeline() {
  clearError();
  try {
    const res = await fetch("/parse", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ code: codeInput.value }),
    });
    const data = await res.json();
    if (!res.ok) {
      showError(data.error || "Parse error");
      return;
    }
    const entries = Object.entries(data.vars || {});
    evalOutput.textContent = entries.map(([k, v]) => `${k} = ${v}`).join("\n");
    codegenOutput.textContent = data.python;
    renderAST(data.ast);
  } catch (err) {
    showError("Server error: " + err.message);
  }
}

codeInput.addEventListener("keydown", (e) => {
  if (e.key === "Tab") {
    e.preventDefault();
    const start = codeInput.selectionStart;
    const end = codeInput.selectionEnd;
    codeInput.value =
      codeInput.value.slice(0, start) + "    " + codeInput.value.slice(end);
    codeInput.selectionStart = codeInput.selectionEnd = start + 4;
  }
});

runBtn.addEventListener("click", runPipeline);

codeInput.value = DEFAULT_CODE;
runPipeline();
