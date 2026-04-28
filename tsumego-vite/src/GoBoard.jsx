import React from 'react';

const GoBoard = ({ initialStones = [], displayedSequence = [], activeLabels = [], onIntersectionClick, isInputting, currentTurn, status}) => {
  const boardSize = 19;
  const cellSize = 5;
  const padding = 5;
  const svgSize = (boardSize - 1) * cellSize + padding * 2;

  const starPoints = [3, 9, 15];

  return (
    <div className={`relative w-full max-w-lg mx-auto ${isInputting ? 'ring-4 ring-blue-500/50 rounded-sm' : ''}`}>
      <svg viewBox={`0 0 ${svgSize} ${svgSize}`} className="w-full h-auto bg-[#e5c07b] shadow-2xl rounded-sm">
        
        {/* 罫線の描画 */}
        {[...Array(boardSize)].map((_, i) => (
          <React.Fragment key={`lines-${i}`}>
            <line x1={padding + i * cellSize} y1={padding} x2={padding + i * cellSize} y2={svgSize - padding} stroke="#5c4033" strokeWidth="0.2" />
            <line x1={padding} y1={padding + i * cellSize} x2={svgSize - padding} y2={padding + i * cellSize} stroke="#5c4033" strokeWidth="0.2" />
          </React.Fragment>
        ))}

        {/* 星の描画 */}
        {starPoints.map(x => 
          starPoints.map(y => (
            <circle key={`star-${x}-${y}`} cx={padding + x * cellSize} cy={padding + y * cellSize} r="0.6" fill="#5c4033" />
          ))
        )}

        {/* 初期配置の石 */}
        {initialStones.map((stone, idx) => (
          <circle key={`init-${idx}`} cx={padding + stone.x * cellSize} cy={padding + stone.y * cellSize} r="2.35" fill={stone.color === 'B' ? '#1a1a1a' : '#f5f5f5'} stroke="#333" strokeWidth="0.1" />
        ))}

        {/* 答え合わせ時の手順描画 */}
        {displayedSequence.map((stone, idx) => (
          <g key={`seq-${idx}`}>
            <circle cx={padding + stone.x * cellSize} cy={padding + stone.y * cellSize} r="2.35" fill={stone.color === 'B' ? '#1a1a1a' : '#f5f5f5'} stroke="#333" strokeWidth="0.1" className="drop-shadow-md" />
            <text x={padding + stone.x * cellSize} y={padding + stone.y * cellSize} textAnchor="middle" dominantBaseline="central" fill={stone.color === 'B' ? '#fff' : '#000'} fontSize="2.5" fontFamily="sans-serif">
              {idx + 1}
            </text>
          </g>
        ))}

        {/* ラベル(LB)の描画 (白い四角の上に黒文字) */}
        {activeLabels.map((label, idx) => (
          <g key={`label-${idx}`}>
            <rect 
              x={padding + label.x * cellSize - 1.8} 
              y={padding + label.y * cellSize - 1.8} 
              width="3.6" 
              height="3.6" 
              fill="rgba(255, 255, 255, 0.95)" 
              stroke="#333" 
              strokeWidth="0.15"
              rx="0.3"
            />
            <text 
              x={padding + label.x * cellSize} 
              y={padding + label.y * cellSize} 
              textAnchor="middle" 
              dominantBaseline="central" 
              fill="#000" 
              fontSize="2.5" 
              fontFamily="sans-serif"
              fontWeight="bold"
            >
              {label.text}
            </text>
          </g>
        ))}

        {/* クリック判定用の透明な交点 */}
        {[...Array(boardSize)].map((_, x) =>
          [...Array(boardSize)].map((_, y) => {
            // --- 【追加】ホバー時のプレビュー石の色を決定するクラス ---
            // 読み入力中（isInputting）の場合のみホバー効果を適用します。
            let hoverClass = "";
            // 結果表示画面(result)以外なら、常に現在の手番のプレビュー石を表示する
            if (status !== 'result') {
              hoverClass = currentTurn === 'B' 
                ? 'hover:fill-black/30' 
                : 'hover:fill-white/50';
            }
            // -----------------------------------------------------

            return (
              <circle
                key={`click-${x}-${y}`}
                cx={padding + x * cellSize}
                cy={padding + y * cellSize}
                r="2.5"
                fill="transparent"
                // 修正前: className="cursor-pointer hover:fill-black/20"
                // 修正後: hoverClass を動的に適用。transition-colors で滑らかに変化させる。
                className={`cursor-pointer transition-colors ${hoverClass}`} 
                onClick={() => onIntersectionClick(x, y)}
              />
            );
          })
        )}
      </svg>
    </div>
  );
};

export default GoBoard;