import React, { useState, useEffect, useRef } from 'react';
import Board from '@sabaki/go-board';
import GoBoard from './GoBoard';

const getBoardHash = (board) => JSON.stringify(board.signMap);

export default function App() {
  // --- 1. ステート管理 ---
  const [problem, setProblem] = useState(null);
  const [currentNode, setCurrentNode] = useState(null);
  const [status, setStatus] = useState('idle');
  const [currentTurn, setCurrentTurn] = useState('B');
  const [mentalSequence, setMentalSequence] = useState([]);
  const [judgment, setJudgment] = useState(null);
  const [activeComment, setActiveComment] = useState(null);
  const [activeLabels, setActiveLabels] = useState([]);
  const [sabakiBoard, setSabakiBoard] = useState(null);
  const [boardHistory, setBoardHistory] = useState([]);
  const [isLoading, setIsLoading] = useState(true);

  // ユーザーのレーティング管理 (初期値1450)
  const [userElo, setUserElo] = useState(1450);

  // StrictModeによる二重リクエスト防止用
  const initialized = useRef(false);

  // --- 2. ユーティリティ関数 ---
  const buildInitialBoard = (initialStones) => {
    const emptySignMap = Array.from({ length: 19 }, () => Array(19).fill(0));
    let board = new Board(emptySignMap);
    initialStones.forEach(st => {
      board = board.set([st.x, st.y], st.color === 'B' ? 1 : -1);
    });
    return board;
  };

  // --- 3. API通信ロジック ---
  
  // 問題の取得
  const fetchNextProblem = async () => {
    setIsLoading(true);
    try {
      // 自分のEloに近い問題をリクエスト
      const response = await fetch(`https://tsumego-infinite.onrender.com/api/problem/next?user_elo=${userElo}`);
      const data = await response.json();
      
      setProblem(data);
      setCurrentNode(data.tree);
      setCurrentTurn(data.startTurn);
      setStatus('idle');
      setMentalSequence([]);
      setJudgment(null);
      setActiveComment(null);
      setActiveLabels([]);

      const initialBoard = buildInitialBoard(data.initialStones);
      setSabakiBoard(initialBoard);
      setBoardHistory([getBoardHash(initialBoard)]);
    } catch (error) {
      console.error("問題の取得に失敗しました", error);
    } finally {
      setIsLoading(false);
    }
  };

  // レーティングの更新
  const updateUserRating = async (isCorrect) => {
    if (!problem) return;

    const payload = {
      problem_id: problem.problem_id,
      user_elo: userElo,
      problem_elo: problem.difficulty,
      is_correct: isCorrect
    };

    try {
      const response = await fetch('https://tsumego-infinite.onrender.com/api/rating/update', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      });
      const data = await response.json();
      
      if (data.new_user_elo) {
        setUserElo(data.new_user_elo);
        // 問題側の新しいEloもUIに反映させたい場合は、ここでproblemステートを更新
        setProblem(prev => ({ ...prev, difficulty: data.new_prob_elo }));
      }
    } catch (error) {
      console.error("レーティングの更新に失敗しました", error);
    }
  };

  // 初回マウント時の実行
  useEffect(() => {
    if (!initialized.current) {
      initialized.current = true;
      fetchNextProblem();
    }
  }, []);

  // --- 4. イベントハンドラー ---
  const handleIntersectionClick = (x, y) => {
    if (status === 'result' || !sabakiBoard) return;

    const sign = currentTurn === 'B' ? 1 : -1;
    let nextBoard;
    try {
      nextBoard = sabakiBoard.makeMove(sign, [x, y], { preventOverwrite: true, preventSuicide: true });
    } catch (e) {
      return; 
    }

    const nextHash = getBoardHash(nextBoard);
    if (boardHistory.includes(nextHash)) return;

    setSabakiBoard(nextBoard);
    setBoardHistory([...boardHistory, nextHash]);
    if (status === 'idle') setStatus('inputting');

    const newMove = { x, y, color: currentTurn };
    const newSequence = [...mentalSequence, newMove];
    setMentalSequence(newSequence);

    const nextNode = currentNode.children?.find(child => child.x === x && child.y === y);

    if (nextNode?.comment) setActiveComment(nextNode.comment);
    else if (nextNode) setActiveComment(null);
    if (nextNode?.labels) setActiveLabels(nextNode.labels);
    else if (nextNode) setActiveLabels([]);

    if (!nextNode) {
      // 不正解確定
      setStatus('result');
      setJudgment(`incorrect_${currentTurn === 'B' ? 'black' : 'white'}`);
      updateUserRating(false);
    } else {
      if (!nextNode.children || nextNode.children.length === 0) {
        // 正解または不正解の終端
        setStatus('result');
        const isCorrect = !!nextNode.is_correct;
        setJudgment(isCorrect ? 'correct' : `incorrect_${currentTurn === 'B' ? 'black' : 'white'}`);
        updateUserRating(isCorrect);
      } else {
        setCurrentNode(nextNode);
        setCurrentTurn(currentTurn === 'B' ? 'W' : 'B');
      }
    }
  };

  const handleReset = () => {
    setStatus('idle');
    setCurrentTurn(problem.startTurn);
    setMentalSequence([]);
    setCurrentNode(problem.tree);
    setJudgment(null);
    setActiveComment(null);
    setActiveLabels([]);
    const initialBoard = buildInitialBoard(problem.initialStones);
    setSabakiBoard(initialBoard);
    setBoardHistory([getBoardHash(initialBoard)]);
  };

  // --- 5. レンダリング ---
  if (isLoading || !problem) {
    return (
      <div className="min-h-screen flex items-center justify-center bg-slate-900 text-slate-100">
        <div className="flex flex-col items-center gap-4">
          <div className="w-8 h-8 border-4 border-blue-500 border-t-transparent rounded-full animate-spin"></div>
          <p className="text-slate-400 font-medium">盤面を読み込み中...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen flex flex-col items-center py-8 px-4 font-sans bg-slate-900 text-slate-100">
      <header className="w-full max-w-lg flex justify-between items-center mb-6 text-sm">
        <div className="flex flex-col">
          <span className="text-slate-400">Your Rank</span>
          <span className="text-xl font-bold text-white">{userElo} Elo</span>
        </div>
        <div className="flex flex-col text-right">
          <span className="text-slate-400">Problem Difficulty</span>
          <span className="text-xl font-bold text-white">{problem.difficulty} Elo</span>
        </div>
      </header>

      <div className="h-16 flex items-center justify-center mb-2">
        {status === 'idle' && <p className="text-lg font-medium text-slate-300">黒番から</p>}
        {status === 'inputting' && (
          <p className="text-lg font-medium text-blue-400 animate-pulse">
            {currentTurn === 'B' ? '黒' : '白'}の読みを入力中... ({mentalSequence.length + 1} 手目)
          </p>
        )}
        {status === 'result' && (
          <div className={`flex items-center gap-2 text-2xl font-bold ${judgment === 'correct' ? 'text-emerald-400' : 'text-rose-400'}`}>
            {judgment === 'correct' ? <>✅ 正解！</> : <>❌ 不正解 ({judgment === 'incorrect_black' ? '黒' : '白'}の手に誤り)</>}
          </div>
        )}
      </div>

      <div className="min-h-16 w-full max-w-lg mb-4 flex flex-col justify-end">
        {status === 'result' && activeComment && (
          <div className="bg-slate-800/80 border-l-4 border-blue-500 p-3 rounded-r-lg shadow-inner">
            <p className="text-sm text-slate-200 leading-relaxed">
              <span className="font-bold text-blue-400 mr-1">解説:</span>
              {activeComment}
            </p>
          </div>
        )}
      </div>

      <GoBoard 
        initialStones={problem.initialStones}
        displayedSequence={status === 'result' ? mentalSequence : []}
        activeLabels={status === 'result' ? activeLabels : []}
        onIntersectionClick={handleIntersectionClick}
        isInputting={status === 'inputting'}
        currentTurn={currentTurn}
        status={status}
      />

      <div className="mt-8 flex gap-4 w-full max-w-lg">
        <button
          onClick={handleReset}
          disabled={status === 'idle' || status === 'result'}
          className="flex-1 flex items-center justify-center gap-2 py-3 px-4 rounded-lg bg-slate-800 text-slate-300 hover:bg-slate-700 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
        >
          🔄 リセット
        </button>
        {status === 'result' && (
          <button 
            onClick={fetchNextProblem} 
            className="flex-1 py-3 px-4 rounded-lg bg-blue-600 text-white font-bold hover:bg-blue-500 transition-colors shadow-lg shadow-blue-900/50"
          >
            次の問題へ
          </button>
        )}
      </div>
    </div>
  );
}