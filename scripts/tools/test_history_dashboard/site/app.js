let state = {
  allTests : [],
  filteredTests : [],
  selectedTest : null,
  granularity : "weekly",
  chart : null,
  page : 1,
  pageSize : 20,
};

function fmtNumber(n)
{
  return new Intl.NumberFormat().format(n ?? 0);
}

function fmtPercent(n)
{
  return `${Number(n ?? 0).toFixed(2)}%`;
}

function fmtDateTime(ms)
{
  if (!ms)
    return "-";
  return new Date(ms).toLocaleString();
}

function fmtDuration(ms)
{
  if (!ms || ms < 0)
    return "-";
  if (ms < 1000)
    return `${ms} ms`;
  if (ms < 60000)
    return `${(ms / 1000).toFixed(2)} s`;
  return `${(ms / 60000).toFixed(2)} min`;
}

function badge(status)
{
  return `<span class="status-badge status-${status}">${status}</span>`;
}

function getEl(id)
{
  return document.getElementById(id);
}

function compareTests(a, b, sortBy)
{
  if (sortBy === "displayName") {
    return a.displayName.localeCompare(b.displayName);
  }
  return (b[sortBy] ?? 0) - (a[sortBy] ?? 0);
}

function applyFilters()
{
  const q            = getEl("searchInput").value.trim().toLowerCase();
  const statusFilter = getEl("statusFilter").value;
  const bucketFilter = getEl("bucketFilter").value;
  const sortBy       = getEl("sortBy").value;

  state.filteredTests = state.allTests.filter((t) => {
    const matchesText = !q || t.displayName.toLowerCase().includes(q) || t.fullName.toLowerCase().includes(q);

    const matchesStatus = statusFilter === "all" || t.lastStatus === statusFilter;

    let matchesBucket = true;
    if (bucketFilter === "nonpassing") {
      matchesBucket = t.nonPassingRuns > 0;
    } else if (bucketFilter === "clean") {
      matchesBucket = t.nonPassingRuns === 0;
    } else if (bucketFilter === "flaky") {
      matchesBucket = t.nonPassingRuns > 0 && t.passedRuns > 0;
    } else if (bucketFilter === "flakyRecent") {
      matchesBucket = (t.last7d && t.last7d.flaky) || (t.last30d && t.last30d.flaky);
    }

    return matchesText && matchesStatus && matchesBucket;
  });

  state.filteredTests.sort((a, b) => compareTests(a, b, sortBy));

  state.page = 1;
  renderTable();

  if (!state.selectedTest && state.filteredTests.length > 0) {
    selectTest(state.filteredTests[0].fullName);
  } else if (state.selectedTest && !state.filteredTests.some((t) => t.fullName === state.selectedTest.fullName)) {
    if (state.filteredTests.length > 0) {
      selectTest(state.filteredTests[0].fullName);
    } else {
      state.selectedTest = null;
      renderDetail();
    }
  }
}

function renderSummary(data)
{
  getEl("totalTests").textContent      = fmtNumber(data.summary.totalTests);
  getEl("totalRuns").textContent       = fmtNumber(data.summary.totalRuns);
  getEl("totalNonPassing").textContent = fmtNumber(data.summary.nonPassingRuns);
  getEl("globalPassRate").textContent  = fmtPercent(data.summary.passRate);
}

function renderTable()
{
  const tbody     = document.querySelector("#testsTable tbody");
  tbody.innerHTML = "";

  const total      = state.filteredTests.length;
  const totalPages = Math.max(1, Math.ceil(total / state.pageSize));

  if (state.page > totalPages) {
    state.page = totalPages;
  }

  const start     = (state.page - 1) * state.pageSize;
  const end       = start + state.pageSize;
  const pageItems = state.filteredTests.slice(start, end);

  for (const t of pageItems) {
    const tr            = document.createElement("tr");
    tr.dataset.fullName = t.fullName;

    tr.innerHTML = `
      <td>
        <div class="test-name">${escapeHtml(t.displayName)}</div>
        <div class="test-sub">${escapeHtml(t.fullName)}</div>
      </td>
      <td>${fmtNumber(t.totalRuns)}</td>
      <td>${fmtNumber(t.failedRuns)}</td>
      <td>${fmtNumber(t.brokenRuns)}</td>
      <td>${fmtNumber(t.nonPassingRuns)}</td>
      <td>${fmtPercent(t.passRate)}</td>
      <td>${badge(t.lastStatus || "unknown")}</td>
      <td>${fmtDateTime(t.lastRun)}</td>
    `;

    tr.addEventListener("click", () => selectTest(t.fullName));
    tbody.appendChild(tr);
  }

  getEl("tableCount").textContent = `${total} tests shown`;

  const pageInfoEl = getEl("pageInfo");
  if (pageInfoEl) {
    pageInfoEl.textContent = `Page ${state.page} / ${totalPages}`;
  }
}

function buildTrendData(test)
{
  const source = state.granularity === "weekly" ? test.weekly : test.monthly;
  const labels = Object.keys(source);
  return {
    labels,
    runs : labels.map((k) => source[k].runs || 0),
    passed : labels.map((k) => source[k].passed || 0),
    failed : labels.map((k) => source[k].failed || 0),
    broken : labels.map((k) => source[k].broken || 0),
    nonPassing : labels.map((k) => source[k].nonPassing || 0),
  };
}

function renderDetail()
{
  const title      = getEl("detailTitle");
  const stats      = getEl("detailStats");
  const recentBody = document.querySelector("#recentRunsTable tbody");

  if (!state.selectedTest) {
    title.textContent    = "Select a test";
    stats.innerHTML      = "";
    recentBody.innerHTML = "";
    if (state.chart) {
      state.chart.destroy();
      state.chart = null;
    }
    return;
  }

  const t           = state.selectedTest;
  title.textContent = t.displayName;

  stats.innerHTML = `
        <div class="detail-stat"><div class="detail-stat-label">Runs</div><div class="detail-stat-value">${
      fmtNumber(t.totalRuns)}</div></div>
        <div class="detail-stat"><div class="detail-stat-label">Failed</div><div class="detail-stat-value">${
      fmtNumber(t.failedRuns)}</div></div>
        <div class="detail-stat"><div class="detail-stat-label">Broken</div><div class="detail-stat-value">${
      fmtNumber(t.brokenRuns)}</div></div>
        <div class="detail-stat"><div class="detail-stat-label">Non-passing</div><div class="detail-stat-value">${
      fmtNumber(t.nonPassingRuns)}</div></div>
        <div class="detail-stat"><div class="detail-stat-label">Pass rate</div><div class="detail-stat-value">${
      fmtPercent(t.passRate)}</div></div>
        <div class="detail-stat"><div class="detail-stat-label">Avg duration</div><div class="detail-stat-value">${
      fmtDuration(t.avgDurationMs)}</div></div>

        <div class="detail-stat">
            <div class="detail-stat-label">Last 7d</div>
            <div class="detail-stat-value">${fmtNumber(t.last7d?.nonPassing || 0)}/${fmtNumber(t.last7d?.runs || 0)}</div>
        </div>
        <div class="detail-stat">
            <div class="detail-stat-label">Last 7d flaky</div>
            <div class="detail-stat-value">${t.last7d?.flaky ? "Yes" : "No"}</div>
        </div>
        <div class="detail-stat">
            <div class="detail-stat-label">Last 30d</div>
            <div class="detail-stat-value">${fmtNumber(t.last30d?.nonPassing || 0)}/${fmtNumber(t.last30d?.runs || 0)}</div>
        </div>
        <div class="detail-stat">
            <div class="detail-stat-label">Last 30d flaky</div>
            <div class="detail-stat-value">${t.last30d?.flaky ? "Yes" : "No"}</div>
        </div>
        <div class="detail-stat">
            <div class="detail-stat-label">Last 7d failure rate</div>
            <div class="detail-stat-value">${fmtPercent(t.last7d?.failureRate || 0)}</div>
        </div>
        <div class="detail-stat">
            <div class="detail-stat-label">Last 30d failure rate</div>
            <div class="detail-stat-value">${fmtPercent(t.last30d?.failureRate || 0)}</div>
        </div>
    `;

  recentBody.innerHTML = "";
  for (const run of t.recentRuns) {
    const tr = document.createElement("tr");
    tr.innerHTML = `
      <td>${fmtDateTime(run.start)}</td>
      <td>${badge(run.status || "unknown")}</td>
      <td>${fmtDuration(run.duration)}</td>
      <td>${run.reportUrl ? `<a href="${escapeHtml(run.reportUrl)}" target="_blank" rel="noopener noreferrer">open</a>` : "-"}</td>
    `;
    recentBody.appendChild(tr);
  }

  const trend = buildTrendData(t);
  const ctx   = getEl("trendChart");

  if (state.chart) {
    state.chart.destroy();
  }

  state.chart = new Chart(ctx, {
    type : "bar",
    data : {
      labels : trend.labels,
      datasets : [
        { label : "Runs", data : trend.runs },
        { label : "Failed", data : trend.failed },
        { label : "Broken", data : trend.broken },
        { label : "Non-passing", data : trend.nonPassing },
      ],
    },
    options : {
      responsive : true,
      maintainAspectRatio : false,
      interaction : { mode : "index", intersect : false },
      scales : {
        x : { stacked : false },
        y : { beginAtZero : true, ticks : { precision : 0 } },
      },
      plugins : {
        legend : { position : "top" },
        title : {
          display : true,
          text : state.granularity === "weekly" ? "Weekly trend" : "Monthly trend",
        },
      },
    },
  });
}

function selectTest(fullName)
{
  state.selectedTest = state.allTests.find((t) => t.fullName === fullName) || null;
  renderDetail();
}

function escapeHtml(str)
{
  return String(str)
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;")
      .replaceAll("'", "&#39;");
}

async function loadData()
{
  const res = await fetch("./data/summary.json", { cache : "no-store" });
  if (!res.ok) {
    throw new Error(`Failed to load summary.json: ${res.status}`);
  }
  const data     = await res.json();
  state.allTests = data.tests || [];
  renderSummary(data);
  applyFilters();
}

function bindEvents()
{
  getEl("searchInput")?.addEventListener("input", applyFilters);
  getEl("statusFilter")?.addEventListener("change", applyFilters);
  getEl("bucketFilter")?.addEventListener("change", applyFilters);
  getEl("sortBy")?.addEventListener("change", applyFilters);
  getEl("reloadButton")?.addEventListener("click", loadData);

  getEl("pageSize")?.addEventListener("change", (e) => {
    state.pageSize = Number(e.target.value);
    state.page     = 1;
    renderTable();
  });

  getEl("prevPage")?.addEventListener("click", () => {
    if (state.page > 1) {
      state.page--;
      renderTable();
    }
  });

  getEl("nextPage")?.addEventListener("click", () => {
    const totalPages = Math.max(1, Math.ceil(state.filteredTests.length / state.pageSize));
    if (state.page < totalPages) {
      state.page++;
      renderTable();
    }
  });

  document.querySelectorAll(".toggle-group button").forEach((btn) => {
    btn.addEventListener("click", () => {
      document.querySelectorAll(".toggle-group button").forEach((b) => b.classList.remove("active"));
      btn.classList.add("active");
      state.granularity = btn.dataset.granularity;
      renderDetail();
    });
  });
}

bindEvents();
loadData().catch((err) => {
  console.error(err);
  alert(`Dashboard failed to load: ${err.message}`);
});
