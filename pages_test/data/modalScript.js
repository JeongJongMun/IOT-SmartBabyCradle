const timerDisplay = document.getElementById("timer");
const timerBox = document.getElementById("timerBox");
const startButton = document.getElementById("start-timer-button");
const cancelButton = document.getElementById("cancel-timer-button");
const alarmMessage = document.getElementById("alarm-message");
const alertMessage = document.getElementById("alert-message");
const modalBg = document.getElementById("modalBg");
const modal = document.getElementById("modal");
const modalBox = document.getElementById("modalBox");
const alertIcon = document.getElementById("alert-icon");
const iconBox = document.getElementById("iconBox");
const confirmButton = document.getElementById("confirm-button");
let countdownTimer;

confirmButton.addEventListener("click", () => {
  alarmMessage.textContent = "특이사항 없음.";
  modal.classList.add("hidden");
  modalBg.classList.add("hidden");
});

function showModal(message, iconClass, animationMode, iconAnime) {
  // Clear previous icons and animations
  iconBox.innerHTML = "";
  modalBg.classList.remove("hidden", "alert-mode", "emergency-mode");
  modal.classList.remove("hidden");
  iconBox.classList.remove("ringing", "anime-bounce");
  alertIcon.classList.remove(
    "fa-poo",
    "fa-baby",
    "fa-circle-exclamation",
    "fa-person-breastfeeding"
  );
  // Set new message and icon
  alertMessage.textContent = message;
  const icon = document.createElement("i");
  icon.className = `fas ${iconClass} text-4xl mt-2`;
  iconBox.appendChild(icon);
  iconBox.classList.add(iconAnime);
  // Add animation and show modal
  modalBg.classList.add(animationMode);
}

// 타이머 시작 버튼 클릭 시
startButton.addEventListener("click", () => {
  const hours = parseInt(document.getElementById("hours").value);
  const minutes = parseInt(document.getElementById("minutes").value);
  const totalSeconds = (hours * 60 + minutes);

  startTimer(totalSeconds);
});

// 타이머 취소 버튼 클릭 시
cancelButton.addEventListener("click", () => {
  timerBox.classList.add("hidden");
  stopTimer();
});

function startTimer(totalSeconds) {
  if (totalSeconds <= 0) {
    timerBox.classList.add("hidden");
    return;
  }
  startButton.classList.add("hidden");
  cancelButton.classList.remove("hidden");
  timerBox.classList.remove("hidden");
  countdownTimer = setInterval(() => {
    if (totalSeconds <= 0) {
      showBabyMealAlert();
      timerBox.classList.add("hidden");
      stopTimer();
    } else {
      setTimerDisplay(totalSeconds);
      totalSeconds--;
    }
  }, 1000);
}

function stopTimer() {
  clearInterval(countdownTimer);
  setTimerDisplay(0);
  startButton.classList.remove("hidden");
  cancelButton.classList.add("hidden");
}

function setTimerDisplay(totalSeconds) {
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;
  timerDisplay.textContent = `${hours
    .toString()
    .padStart(2, "0")}:${minutes.toString().padStart(2, "0")}:${seconds
    .toString()
    .padStart(2, "0")}`;
}

// 아기 배변 알림
function showDiaperChangeAlert() {
  showModal(
    "끙차~아기 기저귀를 갈아주세요!",
    "fa-poo",
    "alert-mode",
    "animate-bounce"
  );
}

// 아기 울음 알림
function showBabyCryAlert() {
  showModal(
    "응애응애. 아기가 울고 있어요.",
    "fa-baby",
    "emergency-mode",
    "ringing"
  );
}

// 아기 탈출 알림
function showBabyEscapeAlert() {
  showModal(
    "아기가 요람에서 벗어났습니다!",
    "fa-circle-exclamation",
    "emergency-mode",
    "ringing"
  );
}

function showBabyMealAlert() {
  showModal(
    "응애 ~ 맘마 주세요!",
    "fa-person-breastfeeding",
    "alert-mode",
    "animate-bounce"
  );
}

// // 아기 배변모드 예시: 5초마다 배변 감지 알림 표시
// setTimeout(showDiaperChangeAlert, 2000);
// setTimeout(showBabyEscapeAlert, 2000);
// setTimeout(showBabyCryAlert, 2000);
// setTimeout(showBabyMealAlert, 2000);